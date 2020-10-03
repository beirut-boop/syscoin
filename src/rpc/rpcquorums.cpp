// Copyright (c) 2017-2020 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <rpc/server.h>
#include <validation.h>

#include <masternode/activemasternode.h>

#include <llmq/quorums.h>
#include <llmq/quorums_blockprocessor.h>
#include <llmq/quorums_debug.h>
#include <llmq/quorums_dkgsession.h>
#include <llmq/quorums_signing.h>
#include <llmq/quorums_signing_shares.h>
#include <rpc/util.h>
#include <net.h>
#include <rpc/blockchain.h>
#include <node/context.h>
void quorum_list_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum list",
            "List of on-chain quorums\n"
            "\nArguments:\n"
            "1. count           (number, optional) Number of quorums to list. Will list active quorums\n"
            "                   if \"count\" is not specified.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

UniValue quorum_list(const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() != 1 && request.params.size() != 2))
        quorum_list_help(request);

    LOCK(cs_main);

    int count = -1;
    if (!request.params[1].isNull()) {
        count = request.params[1].get_int();
        if (count < 0) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "count can't be negative");
        }
    }

    UniValue ret(UniValue::VOBJ);

    for (auto& p : Params().GetConsensus().llmqs) {
        UniValue v(UniValue::VARR);

        auto quorums = llmq::quorumManager->ScanQuorums(p.first, ::ChainActive().Tip(), count > -1 ? count : p.second.signingActiveQuorumCount);
        for (auto& q : quorums) {
            v.push_back(q->qc.quorumHash.ToString());
        }

        ret.pushKV(p.second.name, v);
    }


    return ret;
}

void quorum_info_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum info",
            "Return information about a quorum\n"
            "\nArguments:\n"
            "1. llmqType              (int, required) LLMQ type.\n"
            "2. \"quorumHash\"          (string, required) Block hash of quorum.\n"
            "3. includeSkShare        (boolean, optional) Include secret key share in output.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

UniValue BuildQuorumInfo(const llmq::CQuorumCPtr& quorum, bool includeMembers, bool includeSkShare)
{
    UniValue ret(UniValue::VOBJ);

    ret.pushKV("height", quorum->pindexQuorum->nHeight);
    ret.pushKV("type", quorum->params.name);
    ret.pushKV("quorumHash", quorum->qc.quorumHash.ToString());
    ret.pushKV("minedBlock", quorum->minedBlockHash.ToString());

    if (includeMembers) {
        UniValue membersArr(UniValue::VARR);
        for (size_t i = 0; i < quorum->members.size(); i++) {
            auto& dmn = quorum->members[i];
            UniValue mo(UniValue::VOBJ);
            mo.pushKV("proTxHash", dmn->proTxHash.ToString());
            mo.pushKV("pubKeyOperator", dmn->pdmnState->pubKeyOperator.Get().ToString());
            mo.pushKV("valid", quorum->qc.validMembers[i]);
            if (quorum->qc.validMembers[i]) {
                CBLSPublicKey pubKey = quorum->GetPubKeyShare(i);
                if (pubKey.IsValid()) {
                    mo.pushKV("pubKeyShare", pubKey.ToString());
                }
            }
            membersArr.push_back(mo);
        }

        ret.pushKV("members", membersArr);
    }
    ret.pushKV("quorumPublicKey", quorum->qc.quorumPublicKey.ToString());
    CBLSSecretKey skShare = quorum->GetSkShare();
    if (includeSkShare && skShare.IsValid()) {
        ret.pushKV("secretKeyShare", skShare.ToString());
    }
    return ret;
}

UniValue quorum_info(const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() != 3 && request.params.size() != 4))
        quorum_info_help(request);

    LOCK(cs_main);

    uint8_t llmqType = (uint8_t)request.params[1].get_int();
    if (!Params().GetConsensus().llmqs.count(llmqType)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid LLMQ type");
    }

    uint256 quorumHash = ParseHashV(request.params[2], "quorumHash");
    bool includeSkShare = false;
    if (!request.params[3].isNull()) {
        includeSkShare = request.params[3].get_bool();
    }

    auto quorum = llmq::quorumManager->GetQuorum(llmqType, quorumHash);
    if (!quorum) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "quorum not found");
    }

    return BuildQuorumInfo(quorum, true, includeSkShare);
}

void quorum_dkgstatus_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum dkgstatus",
            "Return the status of the current DKG process.\n"
            "Works only when SPORK_17_QUORUM_DKG_ENABLED spork is ON.\n"
            "\nArguments:\n"
            "1. detail_level         (number, optional, default=0) Detail level of output.\n"
            "                        0=Only show counts. 1=Show member indexes. 2=Show member's ProTxHashes.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

UniValue quorum_dkgstatus(const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() < 1 || request.params.size() > 2)) {
        quorum_dkgstatus_help(request);
    }

    int detailLevel = 0;
    if (!request.params[1].isNull()) {
        detailLevel = request.params[1].get_int();
        if (detailLevel < 0 || detailLevel > 2) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid detail_level");
        }
    }

    llmq::CDKGDebugStatus status;
    llmq::quorumDKGDebugManager->GetLocalDebugStatus(status);

    auto ret = status.ToJson(detailLevel);

    LOCK(cs_main);
    int tipHeight = ::ChainActive().Height();

    UniValue minableCommitments(UniValue::VOBJ);
    UniValue quorumConnections(UniValue::VOBJ);
    NodeContext& node = EnsureNodeContext(request.context);
    if(!node.connman)
        throw JSONRPCError(RPC_CLIENT_P2P_DISABLED, "Error: Peer-to-peer functionality missing or disabled");

    for (const auto& p : Params().GetConsensus().llmqs) {
        auto& params = p.second;

        if (fMasternodeMode) {
            const CBlockIndex* pindexQuorum = ::ChainActive()[tipHeight - (tipHeight % params.dkgInterval)];
            auto allConnections = llmq::CLLMQUtils::GetQuorumConnections(params.type, pindexQuorum, activeMasternodeInfo.proTxHash, false);
            auto outboundConnections = llmq::CLLMQUtils::GetQuorumConnections(params.type, pindexQuorum, activeMasternodeInfo.proTxHash, true);
            std::map<uint256, CAddress> foundConnections;
            node.connman->ForEachNode([&](CNode* pnode) {
                if (!pnode->verifiedProRegTxHash.IsNull() && allConnections.count(pnode->verifiedProRegTxHash)) {
                    foundConnections.emplace(pnode->verifiedProRegTxHash, pnode->addr);
                }
            });
            UniValue arr(UniValue::VARR);
            for (auto& ec : allConnections) {
                UniValue obj(UniValue::VOBJ);
                obj.pushKV("proTxHash", ec.ToString());
                if (foundConnections.count(ec)) {
                    obj.pushKV("connected", true);
                    obj.pushKV("address", foundConnections[ec].ToString());
                } else {
                    obj.pushKV("connected", false);
                }
                obj.pushKV("outbound", outboundConnections.count(ec) != 0);
                arr.push_back(obj);
            }
            quorumConnections.pushKV(params.name, arr);
        }

        llmq::CFinalCommitment fqc;
        if (llmq::quorumBlockProcessor->GetMinableCommitment(params.type, tipHeight, fqc)) {
            UniValue obj(UniValue::VOBJ);
            fqc.ToJson(obj);
            minableCommitments.pushKV(params.name, obj);
        }
    }

    ret.pushKV("minableCommitments", minableCommitments);
    ret.pushKV("quorumConnections", quorumConnections);

    return ret;
}

void quorum_memberof_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum memberof",
            "Checks which quorums the given masternode is a member of.\n"
            "\nArguments:\n"
            "1. \"proTxHash\"                (string, required) ProTxHash of the masternode.\n"
            "2. scanQuorumsCount           (number, optional) Number of quorums to scan for. If not specified,\n"
            "                              the active quorum count for each specific quorum type is used.",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

UniValue quorum_memberof(const JSONRPCRequest& request)
{
    if (request.fHelp || (request.params.size() < 2 || request.params.size() > 3)) {
        quorum_memberof_help(request);
    }

    uint256 protxHash = ParseHashV(request.params[1], "proTxHash");
    int scanQuorumsCount = -1;
    if (!request.params[2].isNull()) {
        scanQuorumsCount = request.params[2].get_int();
        if (scanQuorumsCount <= 0) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid scanQuorumsCount parameter");
        }
    }

    const CBlockIndex* pindexTip;
    {
        LOCK(cs_main);
        pindexTip = ::ChainActive().Tip();
    }

    auto mnList = deterministicMNManager->GetListForBlock(pindexTip);
    auto dmn = mnList.GetMN(protxHash);
    if (!dmn) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "masternode not found");
    }

    UniValue result(UniValue::VARR);

    for (const auto& p : Params().GetConsensus().llmqs) {
        auto& params = p.second;
        size_t count = params.signingActiveQuorumCount;
        if (scanQuorumsCount != -1) {
            count = (size_t)scanQuorumsCount;
        }
        auto quorums = llmq::quorumManager->ScanQuorums(params.type, count);
        for (auto& quorum : quorums) {
            if (quorum->IsMember(dmn->proTxHash)) {
                auto json = BuildQuorumInfo(quorum, false, false);
                json.pushKV("isValidMember", quorum->IsValidMember(dmn->proTxHash));
                json.pushKV("memberIndex", quorum->GetMemberIndex(dmn->proTxHash));
                result.push_back(json);
            }
        }
    }

    return result;
}

void quorum_sign_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum sign",
            "Threshold-sign a message\n"
            "\nArguments:\n"
            "1. llmqType              (int, required) LLMQ type.\n"
            "2. \"id\"                  (string, required) Request id.\n"
            "3. \"msgHash\"             (string, required) Message hash.\n"
            "4. \"quorumHash\"          (string, optional) The quorum identifier.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

void quorum_hasrecsig_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum hasrecsig",
            "Test if a valid recovered signature is present\n"
            "\nArguments:\n"
            "1. llmqType              (int, required) LLMQ type.\n"
            "2. \"id\"                  (string, required) Request id.\n"
            "3. \"msgHash\"             (string, required) Message hash.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

void quorum_getrecsig_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum getrecsig",
            "Get a recovered signature\n"
            "\nArguments:\n"
            "1. llmqType              (int, required) LLMQ type.\n"
            "2. \"id\"                  (string, required) Request id.\n"
            "3. \"msgHash\"             (string, required) Message hash.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

void quorum_isconflicting_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum isconflicting",
            "Test if a conflict exists\n"
            "\nArguments:\n"
            "1. llmqType              (int, required) LLMQ type.\n"
            "2. \"id\"                  (string, required) Request id.\n"
            "3. \"msgHash\"             (string, required) Message hash.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

UniValue quorum_sigs_cmd(const JSONRPCRequest& request)
{
    auto cmd = request.params[0].get_str();
    if (request.fHelp || (request.params.size() != 4)) {
        if (cmd == "sign") {
            if ((request.params.size() < 4) || (request.params.size() > 5)) {
                quorum_sign_help(request);
            }
        } else if (cmd == "hasrecsig") {
            quorum_hasrecsig_help(request);
        } else if (cmd == "getrecsig") {
            quorum_getrecsig_help(request);
        } else if (cmd == "isconflicting") {
            quorum_isconflicting_help(request);
        } else {
            // shouldn't happen as it's already handled by the caller
            throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid cmd");
        }
    }

    uint8_t llmqType = (uint8_t)request.params[1].get_int();
    if (!Params().GetConsensus().llmqs.count(llmqType)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid LLMQ type");
    }

    uint256 id = ParseHashV(request.params[2], "id");
    uint256 msgHash = ParseHashV(request.params[3], "msgHash");

    if (cmd == "sign") {
        if (!request.params[4].isNull()) {
            uint256 quorumHash = ParseHashV(request.params[4], "quorumHash");
            auto quorum = llmq::quorumManager->GetQuorum(llmqType, quorumHash);
            if (!quorum) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "quorum not found");
            }
            llmq::quorumSigSharesManager->AsyncSign(quorum, id, msgHash);
            return true;
        } else {
            return llmq::quorumSigningManager->AsyncSignIfMember(llmqType, id, msgHash);
        }
    } else if (cmd == "hasrecsig") {
        return llmq::quorumSigningManager->HasRecoveredSig(llmqType, id, msgHash);
    } else if (cmd == "getrecsig") {
        llmq::CRecoveredSig recSig;
        if (!llmq::quorumSigningManager->GetRecoveredSigForId(llmqType, id, recSig)) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "recovered signature not found");
        }
        if (recSig.msgHash != msgHash) {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "recovered signature not found");
        }
        return recSig.ToJson();
    } else if (cmd == "isconflicting") {
        return llmq::quorumSigningManager->IsConflicting(llmqType, id, msgHash);
    } else {
        // shouldn't happen as it's already handled by the caller
        throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid cmd");
    }
}

void quorum_selectquorum_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum selectquorum",
            "Returns the quorum that would/should sign a request\n"
            "\nArguments:\n"
            "1. llmqType              (int, required) LLMQ type.\n"
            "2. \"id\"                  (string, required) Request id.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

UniValue quorum_selectquorum(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 3) {
        quorum_selectquorum_help(request);
    }

    uint8_t llmqType = (uint8_t)request.params[1].get_int();
    if (!Params().GetConsensus().llmqs.count(llmqType)) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid LLMQ type");
    }

    uint256 id = ParseHashV(request.params[2], "id");

    UniValue ret(UniValue::VOBJ);

    auto quorum = llmq::quorumSigningManager->SelectQuorumForSigning(llmqType, id);
    if (!quorum) {
        throw JSONRPCError(RPC_MISC_ERROR, "no quorums active");
    }
    ret.pushKV("quorumHash", quorum->qc.quorumHash.ToString());

    UniValue recoveryMembers(UniValue::VARR);
    for (int i = 0; i < quorum->params.recoveryMembers; i++) {
        auto dmn = llmq::quorumSigSharesManager->SelectMemberForRecovery(quorum, id, i);
        recoveryMembers.push_back(dmn->proTxHash.ToString());
    }
    ret.pushKV("recoveryMembers", recoveryMembers);

    return ret;
}

void quorum_dkgsimerror_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum dkgsimerror",
            "This enables simulation of errors and malicious behaviour in the DKG. Do NOT use this on mainnet\n"
            "as you will get yourself very likely PoSe banned for this.\n"
            "\nArguments:\n"
            "1. \"type\"                (string, required) Error type.\n"
            "2. rate                  (number, required) Rate at which to simulate this error type.\n",
    {
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

UniValue quorum_dkgsimerror(const JSONRPCRequest& request)
{
    auto cmd = request.params[0].get_str();
    if (request.fHelp || (request.params.size() != 3)) {
        quorum_dkgsimerror_help(request);
    }

    std::string type = request.params[1].get_str();
    double rate = request.params[2].get_real();

    if (rate < 0 || rate > 1) {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid rate. Must be between 0 and 1");
    }

    llmq::SetSimulatedDKGErrorRate(type, rate);

    return UniValue();
}


void quorum_help(const JSONRPCRequest& request)
{
    RPCHelpMan{"quorum",
            "Set of commands for quorums/LLMQs.\n"
            "To get help on individual commands, use \"help quorum command\".\n"
            "\nAvailable commands:\n"
            "  list              - List of on-chain quorums\n"
            "  info              - Return information about a quorum\n"
            "  dkgsimerror       - Simulates DKG errors and malicious behavior\n"
            "  dkgstatus         - Return the status of the current DKG process\n"
            "  memberof          - Checks which quorums the given masternode is a member of\n"
            "  sign              - Threshold-sign a message\n"
            "  hasrecsig         - Test if a valid recovered signature is present\n"
            "  getrecsig         - Get a recovered signature\n"
            "  isconflicting     - Test if a conflict exists\n"
            "  selectquorum      - Return the quorum that would/should sign a request\n",
    {
        {"command", RPCArg::Type::STR, RPCArg::Optional::NO, "The command to execute"}
    },
    RPCResult{RPCResult::Type::NONE, "", ""},
    RPCExamples{""},
    }.Check(request);
}

UniValue quorum(const JSONRPCRequest& request)
{
    if (request.fHelp && request.params.empty()) {
        quorum_help(request);
    }

    std::string command;
    if (!request.params[0].isNull()) {
        command = request.params[0].get_str();
    }

    if (command == "list") {
        return quorum_list(request);
    } else if (command == "info") {
        return quorum_info(request);
    } else if (command == "dkgstatus") {
        return quorum_dkgstatus(request);
    } else if (command == "memberof") {
        return quorum_memberof(request);
    } else if (command == "sign" || command == "hasrecsig" || command == "getrecsig" || command == "isconflicting") {
        return quorum_sigs_cmd(request);
    } else if (command == "selectquorum") {
        return quorum_selectquorum(request);
    } else if (command == "dkgsimerror") {
        return quorum_dkgsimerror(request);
    }
    JSONRPCRequest jreq(request);
    jreq.params = UniValue();
    quorum_help(jreq);
    return jreq.params;
}

void RegisterQuorumsRPCCommands(CRPCTable &t)
{
// clang-format off
static const CRPCCommand commands[] =
{ //  category              name                      actor (function)
  //  --------------------- ------------------------  -----------------------
    { "evo",                "quorum",                 &quorum,                 {}  },
};
// clang-format on
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
