let BITBOX = require('bitbox-sdk').BITBOX;

let fs = require('fs');
let testDataFileName = "/tmp/test_data.json";

// using legacy account path here because it's used by Bitcoin.com wallet. Change to m/44'/145'/0'/0 if you want compatibility with other wallets
let accountPath = "m/44'/0'/0'/0";

// create separate instances of bitbox for mainnet and testnet
var bitboxMainnet = new BITBOX();
var bitboxTestnet = new BITBOX({'restURL' : "https://trest.bitcoin.com/v2/"});

// creates master node, account node and child nodes testing data based on the provided mnemonic and
// for either testnet or mainnet depending on the flag
function createTestData(mnemonic, testnet) {
	// it actually doesn't matter for what this function does whether it uses tesntet or mainnet bitbox, 
	// but pick the appropriate one nevertheless to prevent any further headaches in case it gets changed
	let bitbox = testnet ? bitboxTestnet : bitboxMainnet;
	
	let seed = bitbox.Mnemonic.toSeed(mnemonic);
	let masterNode = bitbox.HDNode.fromSeed(seed, testnet ? "testnet" : "mainnet");
	
	let accountNode = bitbox.HDNode.derivePath(masterNode, accountPath);
	
	// store master and account node data and a container for child nodes to the network's test data
	var testData = {
		"master_xpriv": bitbox.HDNode.toXPriv(masterNode),
		"master_xpub": bitbox.HDNode.toXPub(masterNode),
		"account_xpriv": bitbox.HDNode.toXPriv(accountNode),
		"account_xpub": bitbox.HDNode.tdoXPub(accountNode),
		"child_nodes" : []
	};
	
	// create 10 child nodes and print their addresses for testing purposes
	for (var i = 0; i < 10; i++) {
		var childNode = bitbox.HDNode.derive(accountNode, i);
		testData["child_nodes"].push({
			"index" : i,
			"private_key" : childNode.keyPair.d.toHex(),
			"cashAddress" : bitbox.HDNode.toCashAddress(childNode),
			"toLegacy" : bitbox.Address.toLegacyAddress(bitbox.HDNode.toCashAddress(childNode)),
			"legacy" : bitbox.HDNode.toLegacyAddress(childNode),
			"toCashAddr" : bitbox.Address.toCashAddress(bitbox.HDNode.toLegacyAddress(childNode)),
		});
		
		console.log(bitbox.HDNode.toCashAddress(childNode));
	}
	
	return testData;
}

// Fetches address details for either testnet or mainnet (depending which bitbox instance is provided) 
// and if there are unspent balances, it builds a testing transaction.
// It returns all the data, so they can be stored and compared with the flutter library output
async function fetchAndBuild(testDataAll, outputAddress) {
	// check if the outputAddress provided is for mainnet or testnet
	if (bitboxMainnet.Address.isMainnetAddress(outputAddress)) {
		network = "mainnet";
		bitbox = bitboxMainnet;
	} else if (bitboxMainnet.Address.isTestnetAddress(outputAddress)) {
		network = "testnet";
		bitbox = bitboxTestnet;
	}
	
	// use the appropriate data from the full testing dataset
	let testData = testDataAll[network];
	
	// placeholder for address details fetching function
	var addressesToFetch = [];
	testData["child_nodes"].forEach(function (childNode) {
		addressesToFetch.push(childNode["cashAddress"]);
	});
	
	// fetch the details of all addresses in the dataset
	let details = await bitbox.Address.details(addressesToFetch);
	
	// placeholder for cumulative balance on all testing addresses
	var totalBalance = 0;
	
	// placeholder for a list of all addresses with balances to fetch utxos of
	var utxosToFetch = [];
	
	// go through all the details of the testing child nodes and find out if they have balance
	for (var i = 0; i < testData["child_nodes"].length; i++) {
		if (testData["child_nodes"][i]["cashAddress"] == details[i]["cashAddress"]) {
			testData["child_nodes"][i]["balance"] = details[i]["balanceSat"];
			totalBalance += details[i]["balanceSat"];
			
			if (details[i]["balanceSat"] > 0) {
				utxosToFetch.push(details[i]["cashAddress"]);
			}
		}
	}

	// if there is a balance at any of the addresses, get their details to compile a testing transaction
	if (totalBalance > 0) {
		// get an account node from the testing account master private key
		let accountNode = bitbox.HDNode.fromXPriv(testData["account_xpriv"]);
		
		// get utxo details based on the list compiled above
		let utxos = await bitbox.Address.utxo(utxosToFetch);
		
		// create a transaction builder for the appropriate network
		let builder = new bitbox.TransactionBuilder(network);
		
		// placeholder for input signatures
		var signatures = [];
		
		// go through all the utxo details to get the input and signature data
		for (var i = 0; i < utxos.length; i++) {
			// first add utxos to the test data under the respective child node
			for (var index = 0; index < testData["child_nodes"].length; index++) {
				if (testData["child_nodes"][index]["cashAddress"] == utxos[i]["cashAddress"]) {
					testData["child_nodes"][index]["utxos"] = utxos[i]["utxos"];
				}
			}
			
			// go through the list of utxos for this address to add them into inputs and signatures
			utxos[i]["utxos"].forEach(function (utxo) {
				// add input to the transaction
				builder.addInput(utxo["txid"], utxo["vout"]);
				
				// add signature. Use private key derived from the account node
				signatures.push({
					"vin" : signatures.length,
					"keyPair" : bitbox.HDNode.toKeyPair(accountNode.derive(i)),
					"originalAmount" : utxo["satoshis"]
				});
			});
		};
		
		// calculate fee for the transaction (signatures array length equals numbers of inputs)
		let byteCount = bitbox.BitcoinCash.getByteCount({ P2PKH: signatures.length }, { P2PKH: 1 });
		
		// substract the fee from total balance on the addresses
		let sendAmount = totalBalance - byteCount;
		
		// add output from the output address provided in the console and calculated amount after the fee
		builder.addOutput(outputAddress, sendAmount);
 
		// sign all inputs
		signatures.forEach(function (signature) {
			builder.sign(signature["vin"], signature["keyPair"], undefined, builder.hashTypes.SIGHASH_ALL, signature["originalAmount"]);
		});
		
		// build the transaction
		let tx = builder.build();
		
		// store the output address and testing transaction String
		testData["output_address"] = outputAddress;
		testData["testing_tx_hex"] = tx.toHex();
	}
	
	// save all the details into the testing data for the appropriate network
	testDataAll[network] = testData;
	
	// return the data (this function doesn't decide when to write them into the testing file
	return testDataAll;
}

function printUsage() {
	console.log("Usage:\n");
	console.log("node create_test_data create");
	console.log("\t# creates testing data for both mainnet and testnet\n");
	console.log("node create_test_data update bchtest:<testnetcashaddress>");
	console.log("\t# checks balances of the generated testnet addresses, \n\t# populates the utxos,\n\t# and generates a testing transaction with the provided output\n")
	console.log("node create_test_data update bitcoincash:<mainnetcashaddress>");
	console.log("\t# checks balances of the generated mainnet addresses, \n\t# populates the utxos,\n\t# and generates a testing transaction with the provided output\n")
	console.log("node create_test_data update bchtest:<testnetcashaddress> bitcoincash:<mainnetcashaddress>");
	console.log("\t# checks balances of the generated mainnet and testnet addresses, \n\t# populates the utxos,\n\t# and generates testing transactions with the provided output")
}

async function exec(arguments) {
	if (arguments.length >= 4 && arguments.length <= 5 && arguments[2] == 'update') {
		// if update command was called, read the testing data and parse them
		let testDataJson = fs.readFileSync(testDataFileName);
		let testData = JSON.parse(testDataJson);
		
		try {
			testData = await fetchAndBuild(testData, arguments[3]);
			
			if (arguments.length == 5) {
				testData = await fetchAndBuild(testData, arguments[4]);
			}
			
			fs.writeFileSync(testDataFileName, JSON.stringify(testData));
		} catch (error) {
			console.log(error);
			console.log("\nError: Invalid output address\n");
			printUsage();
		}
		
		console.log("data stored to " + testDataFileName);
	} else if (arguments.length > 2 & arguments[2] == 'create') {
		// Create testing data from scratch (test file will be rewritten!)
		
		// for generating mnemonic, it doesn't matter which bitbox instance we use
		let mnemonic = bitboxMainnet.Mnemonic.generate();
	
		// create test data map with basic information universal for both networks
		let testData = {
			"mnemonic" : mnemonic,
			"account_path" : accountPath
		};
		
		console.log("\n\nSend any amount to any of the following addresses to test:\n- fetching of address details, balances, utxos;\n- transaction builder\n");
		console.log("You will (should :-) be able to withdraw all deposited amount from the Flutter library at the end of the test.")
		console.log("But just in case if you'll be testing with real coin, backup the test data file as it contains the private keys, which you can use to recover the funds manually.\n");
		
		// generate test data for the mainnet
		testData["mainnet"] = createTestData(mnemonic, false);
		console.log("\n");
		
		// generate test data for the testnet
		testData["testnet"] = createTestData(mnemonic, true);
		
		console.log("\nWhen You're done depositing the testing amounts, wait for the funds to be confirmed in both testnet and mainnet and run 'node create_test_data update' to update the test data with balances, utxos and to generate testing transactions, which will be then compared in Flutter library before broadcasting");
		
		// write the test json file
		fs.writeFileSync(testDataFileName, JSON.stringify(testData));
		
		console.log("\nData stored to " + testDataFileName);
	} else {
		// if it's not clear what the user intended, print the usage details
		printUsage();
	}
}

exec(process.argv);