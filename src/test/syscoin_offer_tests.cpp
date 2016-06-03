#include "test/test_syscoin_services.h"
#include "utiltime.h"
#include "rpcserver.h"
#include "alias.h"
#include <boost/test/unit_test.hpp>
BOOST_GLOBAL_FIXTURE( SyscoinTestingSetup );

BOOST_FIXTURE_TEST_SUITE (syscoin_offer_tests, BasicSyscoinTestingSetup)


BOOST_AUTO_TEST_CASE (generate_offernew)
{
	printf("Running generate_offernew...\n");
	UniValue r;

	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "selleralias1", "changeddata1");

	// generate a good offer
	string offerguid = OfferNew("node1", "selleralias1", "category", "title", "100", "0.05", "description", "USD");

	// should fail: generate an offer with unknown alias
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES fooalias category title 100 0.05 description USD"), runtime_error);
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck fooalias category title 100 0.05 description USD"), runtime_error);

	// should fail: generate an offer with negative quantity
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES selleralias1 category title -2 0.05 description USD"), runtime_error);
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck selleralias1 category title -2 0.05 description USD"), runtime_error);

	// should fail: generate an offer with zero price
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES selleralias1 category title 100 0 description USD"), runtime_error);
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck selleralias1 category title 100 0 description USD"), runtime_error);

	// should fail: generate an offer with negative price
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES selleralias1 category title 100 -0.05 description USD"), runtime_error);
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck selleralias1 category title 100 -0.05 description USD"), runtime_error);

	// should fail: generate an offer too-large category
	string s256bytes = "SfsdfdfsdsfSfsdfdfsdsfDsdsdsdsfsfsdsfsdsfdsfsdsfdsfsdsfsdSfsdfdfsdsfSfsdfdfsdsfDsdsdsdsfsfsdsfsdsfdsfsdsfdsfsdsfsdSfsdfdfsdsfSfsdfdfsdsfDsdsdsdsfsfsdsfsdsfdsfsdsfdsfsdsfsdSfsdfdfsdsfSfsdfdfsdsfDsdsdsdsfsfsdsfsdsfdsfsdsfdsfsdsfsdSfsdfdfsdsfSfsdfdfsdsDfdfddz";
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES selleralias1 " + s256bytes + " title 100 0.05 description USD"), runtime_error);	
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck selleralias1 " + s256bytes + " title 100 0.05 description USD"), runtime_error);	

	// should fail: generate an offer too-large title
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES selleralias1 category " + s256bytes + " 100 0.05 description USD"), runtime_error);	
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck selleralias1 category " + s256bytes + " 100 0.05 description USD"), runtime_error);

	// should fail: generate an offer too-large description
	string s1024bytes =   "asdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfssdsfsdfsdfsdfsdfsdsdfdfsdfsdfsdfsdz";
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES selleralias1 category title 100 0.05 " + s1024bytes + " USD"), runtime_error);
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck selleralias1 category title 100 0.05 " + s1024bytes + " USD"), runtime_error);

	// should fail: generate an offer with invalid currency
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES selleralias1 category title 100 0.05 description ZZZ"), runtime_error);
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck selleralias1 category title 100 0.05 description ZZZ"), runtime_error);
}

BOOST_AUTO_TEST_CASE (generate_certoffernew)
{
	printf("Running generate_certoffernew...\n");
	UniValue r;

	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "node1alias", "node1aliasdata");
	AliasNew("node2", "node2alias", "node2aliasdata");

	string certguid1  = CertNew("node1", "node1alias", "title", "data");
	string certguid1a = CertNew("node1", "node1alias", "title", "data");
	string certguid2  = CertNew("node2", "node2alias", "title", "data");

	// generate a good cert offer
	string offerguid = OfferNew("node1", "node1alias", "category", "title", "1", "0.05", "description", "USD", certguid1);

	// should fail: generate a cert offer using a quantity greater than 1
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offernew SYS_RATES node1alias category title 2 0.05 description USD " + certguid1a));
	const UniValue &arr = r.get_array();
	string guid = arr[1].get_str();
	GenerateBlocks(10, "node1");
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + guid));
	BOOST_CHECK(find_value(r.get_obj(), "quantity").get_str() == "1");

	// should fail: generate a cert offer using a zero quantity
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES node1alias category title 0 0.05 description USD " + certguid1a), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offernew_nocheck SYS_RATES node1alias category title 0 0.05 description USD " + certguid1a));
		const UniValue &arr1 = r.get_array();
		guid = arr1[1].get_str();
		GenerateBlocks(10, "node1");
		BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + guid));
		BOOST_CHECK(find_value(r.get_obj(), "quantity").get_str() == "1");
	#endif
	// should fail: generate a cert offer using an unlimited quantity
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES node1alias category title -1 0.05 description USD " + certguid1a), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offernew_nocheck SYS_RATES node1alias category title -1 0.05 description USD " + certguid1a));
		const UniValue &arr2 = r.get_array();
		guid = arr2[1].get_str();
		GenerateBlocks(10, "node1");
		BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + guid));
		BOOST_CHECK(find_value(r.get_obj(), "quantity").get_str() == "1");
	#endif
	// should fail: generate a cert offer using a cert guid you don't own
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES node1alias category title 1 0.05 description USD " + certguid2), runtime_error);	
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck node1alias category title 1 0.05 description USD " + certguid2), runtime_error);	
	#endif
	// should fail: generate a cert offer if accepting only BTC
	BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES node1alias category title 1 0.05 description USD " + certguid1a + " 0 1"), runtime_error);

	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck node1alias category title 1 0.05 description USD " + certguid1a + " 0 1"), runtime_error);
		// should fail: generate a cert offer using different public keys for cert and alias
		BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck node1alias category title 1 0.05 description USD " + certguid1a + " 0 0 1"), runtime_error);
	#endif

}

BOOST_AUTO_TEST_CASE (generate_offernew_linkedoffer)
{
	printf("Running generate_offernew_linkedoffer...\n");
	UniValue r;

	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "selleralias5", "changeddata1");
	AliasNew("node2", "selleralias6", "changeddata1");

	// generate a good offer
	string offerguid = OfferNew("node1", "selleralias5", "category", "title", "100", "10.00", "description", "USD", "/""/", false);
	string lofferguid = OfferLink("node2", "selleralias6", offerguid, "5", "newdescription");
	BOOST_CHECK_NO_THROW(r = CallRPC("node2", "offerinfo " + lofferguid));
	BOOST_CHECK_EQUAL(find_value(r.get_obj(), "price").get_str(), "10.50");

	// generate a cert offer using a negative percentage
	BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink selleralias6 " + offerguid + " -5 newdescription"), runtime_error);	
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink_nocheck selleralias6 " + offerguid + " -5 newdescription"), runtime_error);	
	#endif

	// should fail: generate a cert offer using too-large pergentage
	BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink selleralias6 " + offerguid + " 256 newdescription"), runtime_error);	
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink_nocheck selleralias6 " + offerguid + " 256 newdescription"), runtime_error);	
	#endif
	// should fail: generate an offerlink with too-large description
	string s1024bytes =   "asdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfssdsfsdfsdfsdfsdfsdsdfdfsdfsdfsdfsdz";
	BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink selleralias6 " + offerguid + " 5 " + s1024bytes), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink_nocheck selleralias6 " + offerguid + " 5 " + s1024bytes), runtime_error);
	#endif
}

BOOST_AUTO_TEST_CASE (generate_offernew_linkedofferexmode)
{
	printf("Running generate_offernew_linkedofferexmode...\n");
	UniValue r;

	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "selleralias8", "changeddata1");
	AliasNew("node2", "selleralias9", "changeddata1");

	// generate a good offer in exclusive mode
	string offerguid = OfferNew("node1", "selleralias8", "category", "title", "100", "0.05", "description", "USD");

	// should fail: attempt to create a linked offer for an exclusive mode product without being on the whitelist
	BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink selleralias9 " + offerguid + " 5 newdescription"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink_nocheck selleralias9 " + offerguid + " 5 newdescription"), runtime_error);
	#endif

	// should succeed: offer seller adds affiliate to whitelist
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offeraddwhitelist " + offerguid + " selleralias9 10"));
	GenerateBlocks(10);

	// should succeed: attempt to create a linked offer for an exclusive mode product while being on the whitelist
	OfferLink("node2", "selleralias9", offerguid, "5", "newdescription");
}

BOOST_AUTO_TEST_CASE (generate_offernew_linkedlinkedoffer)
{
	printf("Running generate_offernew_linkedlinkedoffer...\n");
	UniValue r;

	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "selleralias12", "changeddata1");
	AliasNew("node2", "selleralias13", "changeddata1");
	AliasNew("node3", "selleralias14", "changeddata1");

	// generate a good offer
	string offerguid = OfferNew("node1", "selleralias12", "category", "title", "100", "0.05", "description", "USD", "/""/", false);
	string lofferguid = OfferLink("node2", "selleralias13", offerguid, "5", "newdescription");

	// should fail: try to generate a linked offer with a linked offer
	BOOST_CHECK_THROW(r = CallRPC("node3", "offerlink selleralias14 " + lofferguid + " 5 newdescription"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node3", "offerlink_nocheck selleralias14 " + lofferguid + " 5 newdescription"), runtime_error);	
	#endif
}

BOOST_AUTO_TEST_CASE (generate_offerupdate)
{
	printf("Running generate_offerupdate...\n");
	UniValue r;
	
	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "selleralias2", "changeddata1");

	// generate a good offer
	string offerguid = OfferNew("node1", "selleralias2", "category", "title", "100", "0.05", "description", "USD");

	// perform a valid update
	OfferUpdate("node1", "selleralias2", offerguid, "category", "titlenew", "90", "0.15", "descriptionnew");

	// should fail: offer cannot be updated by someone other than owner
	BOOST_CHECK_THROW(r = CallRPC("node2", "offerupdate SYS_RATES selleralias2 " + offerguid + " category title 90 0.15 description"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offerupdate_nocheck selleralias2 " + offerguid + " category title 90 0.15 description"), runtime_error);
	#endif

	// should fail: generate an offer with unknown alias
	BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate SYS_RATES fooalias " + offerguid + " category title 90 0.15 description"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate_nocheck fooalias " + offerguid + " category title 90 0.15 description"), runtime_error);
	#endif

	// should fail: generate an offer with zero price
	BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate SYS_RATES selleralias2 " + offerguid + " category title 90 0 description"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate_nocheck selleralias2 " + offerguid + " category title 90 0 description"), runtime_error);
	#endif
	// should fail: generate an offer with negative price
	BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate SYS_RATES selleralias2 " + offerguid + " category title 90 -0.05 description"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate_nocheck selleralias2 " + offerguid + " category title 90 -0.05 description"), runtime_error);
	#endif

	// should fail: generate an offer too-large category
	string s256bytes =   "SfsdfdfsdsfSfsdfdfsdsfDsdsdsdsfsfsdsfsdsfdsfsdsfdsfsdsfsdSfsdfdfsdsfSfsdfdfsdsfDsdsdsdsfsfsdsfsdsfdsfsdsfdsfsdsfsdSfsdfdfsdsfSfsdfdfsdsfDsdsdsdsfsfsdsfsdsfdsfsdsfdsfsdsfsdSfsdfdfsdsfSfsdfdfsdsfDsdsdsdsfsfsdsfsdsfdsfsdsfdsfsdsfsdSfsdfdfsdsfSfsdfdfsdsDfdfddz";
	BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate SYS_RATES selleralias2 " + offerguid + " " + s256bytes + " title 90 0.15 description"), runtime_error);	
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate_nocheck selleralias2 " + offerguid + " " + s256bytes + " title 90 0.15 description"), runtime_error);	
	#endif
	// should fail: generate an offer too-large title
	BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate SYS_RATES selleralias2 " + offerguid + " category " + s256bytes + " 90 0.15 description"), runtime_error);	
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate_nocheck selleralias2 " + offerguid + " category " + s256bytes + " 90 0.15 description"), runtime_error);	
	#endif
	// should fail: generate an offer too-large description
	string s1024bytes =   "asdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfssdsfsdfsdfsdfsdfsdsdfdfsdfsdfsdfsdz";
	BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate SYS_RATES selleralias2 " + offerguid + " category title 90 0.15 " + s1024bytes), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate_nocheck selleralias2 " + offerguid + " category title 90 0.15 " + s1024bytes), runtime_error);
	#endif
}

BOOST_AUTO_TEST_CASE (generate_offeraccept)
{
	printf("Running generate_offeraccept...\n");
	UniValue r;
	
	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "selleralias3", "somedata");
	AliasNew("node2", "buyeralias3", "somedata");

	// generate a good offer
	string offerguid = OfferNew("node1", "selleralias3", "category", "title", "100", "0.01", "description", "USD");

	// perform a valid accept
	string acceptguid = OfferAccept("node1", "node2", "buyeralias3", offerguid, "1", "message");

	// should fail: generate an offer accept with too-large message
	string s1024bytes =   "asdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfasdfasdfsadfsadassdsfsdfsdfsdfsdfsdsdfssdsfsdfsdfsdfsdfsdsdfdfsdfsdfsdfsdz";
	BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept buyeralias3 " + offerguid + " 1 " + s1024bytes), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept_nocheck buyeralias3 " + offerguid + " 1 " + s1024bytes), runtime_error);
	#endif
	// perform an accept on more items than available
	BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept buyeralias3 " + offerguid + " 100 message"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept_nocheck buyeralias3 " + offerguid + " 100 message"), runtime_error);
	#endif

	// perform an accept on negative quantity
	BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept buyeralias3 " + offerguid + " -1 message"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept_nocheck buyeralias3 " + offerguid + " -1 message"), runtime_error);
	#endif
	// perform an accept on zero quantity
	BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept buyeralias3 " + offerguid + " 0 message"), runtime_error);
	#ifdef ENABLE_DEBUGRPC
		BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept_nocheck buyeralias3 " + offerguid + " 0 message"), runtime_error);
	#endif
}

BOOST_AUTO_TEST_CASE (generate_offerexpired)
{
	printf("Running generate_offerexpired...\n");
	UniValue r;
	
	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "selleralias4", "somedata");
	AliasNew("node2", "buyeralias4", "somedata");

	// generate a good offer
	string offerguid = OfferNew("node1", "selleralias4", "category", "title", "100", "0.01", "description", "USD");

	// this will expire the offer
	GenerateBlocks(100);
	#ifdef ENABLE_DEBUGRPC
		// should fail: perform an accept on expired offer
		BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept buyeralias4 " + offerguid + " 1 message"), runtime_error);
		BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept_nocheck buyeralias4 " + offerguid + " 1 message"), runtime_error);
		// should fail: offer update on an expired offer
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate SYS_RATES selleralias4 " + offerguid + " category title 90 0.15 description"), runtime_error);
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate_nocheck selleralias4 " + offerguid + " category title 90 0.15 description"), runtime_error);
		// should fail: link to an expired offer
		BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink buyeralias4 " + offerguid + " 5 newdescription"), runtime_error);	
		BOOST_CHECK_THROW(r = CallRPC("node2", "offerlink_nocheck buyeralias4 " + offerguid + " 5 newdescription"), runtime_error);	
	#endif
}

BOOST_AUTO_TEST_CASE (generate_offerexpiredexmode)
{
	printf("Running generate_offerexpiredexmode...\n");
	UniValue r;

	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "selleralias10", "changeddata1");
	AliasNew("node2", "selleralias11", "changeddata1");

	// generate a good offer in exclusive mode
	string offerguid = OfferNew("node1", "selleralias10", "category", "title", "100", "0.05", "description", "USD", "", true);

	// should succeed: offer seller adds affiliate to whitelist
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offeraddwhitelist " + offerguid + " selleralias11 10"));

	// this will expire the offer
	GenerateBlocks(100);
	#ifdef ENABLE_DEBUGRPC
		// should fail: remove whitelist item from expired offer
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerremovewhitelist " + offerguid + " selleralias11"), runtime_error);
		// should fail: clear whitelist from expired offer
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerclearwhitelist " + offerguid), runtime_error);
	#endif
}

BOOST_AUTO_TEST_CASE (generate_certofferexpired)
{
	printf("Running generate_certofferexpired...\n");
	UniValue r;

	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node1", "node1alias2", "node1aliasdata");
	AliasNew("node2", "node2alias2", "node2aliasdata");

	string certguid  = CertNew("node1", "node1alias2", "title", "data");

	// this leaves 50 blocks remaining before cert expires
	GenerateBlocks(40);

	// generate a good cert offer - after this, 40 blocks to cert expire
	string offerguid = OfferNew("node1", "node1alias2", "category", "title", "1", "0.05", "description", "USD", certguid);

	// this expires the cert but not the offer
	GenerateBlocks(50);
	#ifdef ENABLE_DEBUGRPC
		// should fail: offer update on offer with expired cert
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate SYS_RATES node1alias2 " + offerguid + " category title 1 0.05 newdescription"), runtime_error);
		BOOST_CHECK_THROW(r = CallRPC("node1", "offerupdate_nocheck SYS_RATES node1alias2 " + offerguid + " category title 1 0.05 newdescription"), runtime_error);
		// should fail: offer accept on offer with expired cert
		BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept node2alias2 " + offerguid + " 1 message"), runtime_error);
		BOOST_CHECK_THROW(r = CallRPC("node2", "offeraccept_nocheck node2alias2 " + offerguid + " 1 message"), runtime_error)	;
		// should fail: generate a cert offer using an expired cert
		BOOST_CHECK_THROW(r = CallRPC("node1", "offernew SYS_RATES node1alias2 category title 1 0.05 description USD " + certguid), runtime_error);
		BOOST_CHECK_THROW(r = CallRPC("node1", "offernew_nocheck node1alias2 category title 1 0.05 description USD " + certguid), runtime_error);	
	#endif
}

BOOST_AUTO_TEST_CASE (generate_offerlink_offlinenode)
{
	printf("Running generate_offerlink_offlinenode...\n");
	UniValue r;

	GenerateBlocks(200);
	GenerateBlocks(200, "node2");
	GenerateBlocks(200, "node3");

	AliasNew("node2", "selleralias15", "changeddata1");
	AliasNew("node1", "selleralias16", "changeddata1");

	// generate a good offer
	string offerguid = OfferNew("node2", "selleralias15", "category", "title", "100", "10.00", "description", "USD", "/""/", false);

	// stop node2 and link offer on node1 while node2 is offline
	StopNode("node2");
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerlink selleralias16 " + offerguid + " 5 newdescription"));
	const UniValue &arr = r.get_array();
	string lofferguid = arr[1].get_str();
	// generate 10 blocks on node1
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "generate 10"));

	// startup node1 again and see that it has linked offer and its right price (with markup)
	StartNode("node2");
	GenerateBlocks(10);
	BOOST_CHECK_NO_THROW(r = CallRPC("node2", "offerinfo " + lofferguid));
	BOOST_CHECK_EQUAL(find_value(r.get_obj(), "price").get_str(), "10.50");
}
BOOST_AUTO_TEST_CASE (generate_offersafesearch)
{
	printf("Running generate_offersafesearch...\n");
	UniValue r;
	GenerateBlocks(1);
	// offer is safe to search
	string offerguidsafe = OfferNew("node2", "selleroffer15", "category", "title", "100", "10.00", "description", "USD", "/""/", true, "0", "location", "Yes");
	// not safe to search
	string offerguidnotsafe = OfferNew("node2", "selleroffer15", "category", "title", "100", "10.00", "description", "USD", "/""/", true, "0", "location", "No");
	// should include result in both safe search mode on and off
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidsafe, "Yes"), true);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidsafe, "No"), true);

	// should only show up if safe search is off
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidnotsafe, "Yes"), false);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidnotsafe, "No"), true);

	// shouldn't affect offerinfo
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + offerguidsafe));
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + offerguidnotsafe));


}
BOOST_AUTO_TEST_CASE (generate_offerban)
{
	printf("Running generate_offerban...\n");
	UniValue r;
	GenerateBlocks(1);
	// offer is safe to search
	string offerguidsafe = OfferNew("node2", "selleroffer15", "category", "title", "100", "10.00", "description", "USD", "/""/", true, "0", "location", "Yes");
	// not safe to search
	string offerguidnotsafe = OfferNew("node2", "selleroffer15", "category", "title", "100", "10.00", "description", "USD", "/""/", true, "0", "location", "No");
	// can't ban on any other node than one that created SYS_BAN
	BOOST_CHECK_THROW(OfferBan("node2",offerguidnotsafe,SAFETY_LEVEL1), runtime_error);
	BOOST_CHECK_THROW(OfferBan("node3",offerguidsafe,SAFETY_LEVEL1), runtime_error);
	// ban both offers level 1 (only owner of SYS_CATEGORY can do this)
	BOOST_CHECK_NO_THROW(OfferBan("node1",offerguidsafe,SAFETY_LEVEL1));
	BOOST_CHECK_NO_THROW(OfferBan("node1",offerguidnotsafe,SAFETY_LEVEL1));
	// should only show level 1 banned if safe search filter is not used
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidsafe, "Yes"), false);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidsafe, "No"), true);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidnotsafe, "Yes"), false);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidnotsafe, "No"), true);
	// should be able to offerinfo on level 1 banned offers
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + offerguidsafe));
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + offerguidnotsafe));
	
	// ban both offers level 2 (only owner of SYS_CATEGORY can do this)
	BOOST_CHECK_NO_THROW(OfferBan("node1",offerguidsafe,SAFETY_LEVEL2));
	BOOST_CHECK_NO_THROW(OfferBan("node1",offerguidnotsafe,SAFETY_LEVEL2));
	// no matter what filter won't show banned offers
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidsafe, "Yes"), false);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidsafe, "No"), false);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidnotsafe, "Yes"), false);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidnotsafe, "No"), false);

	// shouldn't be able to offerinfo on level 2 banned offers
	BOOST_CHECK_THROW(r = CallRPC("node1", "offerinfo " + offerguidsafe), runtime_error);
	BOOST_CHECK_THROW(r = CallRPC("node1", "offerinfo " + offerguidnotsafe), runtime_error);

	// unban both offers (only owner of SYS_CATEGORY can do this)
	BOOST_CHECK_NO_THROW(OfferBan("node1",offerguidsafe,0));
	BOOST_CHECK_NO_THROW(OfferBan("node1",offerguidnotsafe,0));
	// safe to search regardless of filter
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidsafe, "Yes"), true);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidsafe, "No"), true);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidnotsafe, "Yes"), true);
	BOOST_CHECK_EQUAL(OfferFilter("node1", offerguidnotsafe, "No"), true);

	// should be able to offerinfo on non banned offers
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + offerguidsafe));
	BOOST_CHECK_NO_THROW(r = CallRPC("node1", "offerinfo " + offerguidnotsafe));
	
}
BOOST_AUTO_TEST_SUITE_END ()