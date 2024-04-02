import {getTrustedHTML} from 'chrome://resources/js/static_types.js';
import {getRequiredElement} from 'chrome://resources/js/util.js';

import type {SolanaAccountMeta} from './solana_wallet.mojom-webui.js';

function addAccountInfo(parent: HTMLElement,account:SolanaAccountMeta) {
  const div = document.createElement('div');
  div.className = 'client';  

  const buttonDiv = document.createElement('div');
  const overwriteText = document.createElement('label');
  overwriteText.innerHTML = getTrustedHTML`0xAddr `;
  buttonDiv.appendChild(overwriteText);
  const overwriteValue = document.createElement('input');
  overwriteValue.type = 'number';
  overwriteValue.value = '0x12345678';
  overwriteValue.className = 'overwrite';
  buttonDiv.appendChild(overwriteValue);
  
  
  const resultDiv = document.createElement('div');
  resultDiv.textContent = 'Result: ' + String(account.pubkey) + String(account.isSigner);

  parent.appendChild(div);
}



function initialize() {
  addAccountInfo(getRequiredElement('account-info'),{pubkey:'Fg6PaF8sFnSsd9MKz9ZNsaC2zQyrZuEEvRMnPb6Nk6HF',addrTableLookupIndex:null,isSigner:true,isWritable:true});
  
}

document.addEventListener('DOMContentLoaded', initialize);