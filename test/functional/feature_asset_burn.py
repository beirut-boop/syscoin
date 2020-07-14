#!/usr/bin/env python3
# Copyright (c) 2019-2020 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import SyscoinTestFramework
from test_framework.util import assert_equal, assert_raises_rpc_error


class AssetBurnTest(SyscoinTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.rpc_timeout = 240
        self.extra_args = [['-assetindex=1'],['-assetindex=1']]

    def run_test(self):
        self.nodes[0].generate(200)
        self.sync_blocks()
        self.basic_burn_syscoin()
        self.basic_audittxroot1()
    
    def basic_burn_syscoin(self):
        self.basic_asset()
        self.nodes[0].generate(1)
        newaddress = self.nodes[0].getnewaddress()
        self.nodes[0].assetsend(self.asset, newaddress, '0.5')
        self.nodes[0].generate(1)
        out =  self.nodes[0].listunspent(query_options={'assetGuid': self.asset})
        assert_equal(len(out), 1)
        # try to burn more than we own
        assert_raises_rpc_error(-20, 'Failed to read from asset DB', self.nodes[0].assetallocationburn(self.asset, newaddress, '0.6', '0x931d387731bbbc988b312206c74f77d004d6b84b'))
        self.nodes[0].assetallocationburn(self.asset, newaddress, '0.5', '0x931d387731bbbc988b312206c74f77d004d6b84b')
        self.nodes[0].generate(1)
        out =  self.nodes[0].listunspent(query_options={'assetGuid': self.asset})
        assert_equal(len(out), 0)

    def basic_asset(self):
        self.asset = self.nodes[0].assetnew('1', 'TST', 'asset description', '0x9f90b5093f35aeac5fbaeb591f9c9de8e2844a46', 8, '1000', '10000', 31, {})['asset_guid']



if __name__ == '__main__':
    AssetBurnTest().main()