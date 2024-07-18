import { chain } from './chain';
import { client } from './client';
import { inAppWallet } from 'thirdweb/wallets';

export async function loadWallet(token: string, encryptionKey: string) {
  const wallet = inAppWallet();

  return wallet.connect({
    client,
    chain,
    strategy: 'jwt' as const,
    jwt: token,
    encryptionKey: encryptionKey,
  });
}
