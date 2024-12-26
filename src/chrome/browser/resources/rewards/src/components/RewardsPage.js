import React, { useState } from 'react';
import { createThirdwebClient } from 'thirdweb';
import { ConnectButton, useConnect } from 'thirdweb/react';
import { createWallet } from 'thirdweb/wallets';
import { useNavigate } from 'react-router-dom';
import wootzLogo from '../images/wootz.png'

const RewardsPage = ({ onLogout }) => {
  const [isNewTabAdsEnabled, setIsNewTabAdsEnabled] = useState(false);
  const [isNotificationAdsEnabled, setIsNotificationAdsEnabled] = useState(true);
  const [isWootzRewardsEnabled, setIsWootzRewardsEnabled] = useState(false);

  const navigate = useNavigate();
  const connect = useConnect();
  const client = createThirdwebClient({ clientId: "12332434234" });

  const handleGetProfile = () => navigate('/profile');

  const ToggleButton = ({ isEnabled, setIsEnabled, label }) => (
    <div className="flex justify-between items-center">
      <span>{label}: 0</span>
      <button
        onClick={() => setIsEnabled(!isEnabled)}
        className={`w-10 h-6 ${isEnabled ? 'bg-blue-500' : 'bg-gray-300'} rounded-full flex items-center transition-colors duration-200 ease-in-out focus:outline-none`}
      >
        <div className={`w-5 h-5 bg-white rounded-full shadow-md transform transition-transform duration-200 ease-in-out ${isEnabled ? 'translate-x-5' : 'translate-x-0'}`}></div>
      </button>
    </div>
  );

  return (
    <div className="flex flex-col min-h-screen bg-white text-black">
      <header className="sticky top-0 bg-white z-10 p-4 border-b flex justify-center">
        <img src={wootzLogo} alt="WootzApp" className="w-20 h-12" />
      </header>

      <main className="flex-grow overflow-y-auto">
        <div className="max-w-2xl mx-auto p-4 pb-20">
          <h1 className="text-2xl font-bold mb-2 text-center">Manage WootzApp Rewards</h1>
          <p className="mb-4 text-center">
            Control what kinds of WootzApp Rewards you see and how often.
            <a href="#" className="text-blue-600 underline ml-1">Learn more</a>
          </p>

          {/* Partner Rewards Section */}
          <section className="mb-6">
            <h2 className="text-xl font-bold mb-2">Partner Rewards</h2>
            <div className="bg-blue-50 rounded-lg p-4">
              <h3 className="font-semibold">Artifact</h3>
              <p>Artifact Points: 1000</p>
              <p className="mb-2">Artifact Wallet: 50</p>
              <button className="w-full bg-black text-white py-2 rounded-md mb-2">
                Watch An Ad
              </button>
              <p className="mb-1">
                Refer A Friend Link: <a href="https://refer/11211" className="text-blue-600 underline">https://refer/11211</a>
              </p>
              <a href="#" className="text-blue-600 underline">See other offers from Artifact</a>
            </div>
          </section>

          {/* Connect Partners Section */}
          <section className="mb-6">
            <h2 className="text-xl font-semibold mb-4">Connect other partners</h2>
            <div className="flex ">
              <ConnectButton
                client={client}
                wallets={[
                  createWallet("io.metamask"),
                  createWallet("com.coinbase.wallet"),
                  createWallet("me.rainbow"),
                ]}
              />
            </div>
          </section>

          {/* Ads Received Section */}
          <section>
            <h2 className="text-xl font-semibold mb-2">Total ads received this month</h2>
            <div className="space-y-2">
              <ToggleButton isEnabled={isNewTabAdsEnabled} setIsEnabled={setIsNewTabAdsEnabled} label="New tab page ads" />
              <ToggleButton isEnabled={isNotificationAdsEnabled} setIsEnabled={setIsNotificationAdsEnabled} label="Notification ads" />
              <ToggleButton isEnabled={isWootzRewardsEnabled} setIsEnabled={setIsWootzRewardsEnabled} label="WootzApp Rewards" />
            </div>
          </section>
        </div>
        </main>

        <footer className="bg-white mt-auto">
        <div className="grid grid-cols-2">
          <button
            className="bg-black text-white py-4 text-center hover:bg-gray-800 transition-colors duration-200"
            onClick={handleGetProfile}
          >
            Get Profile
          </button>
          <button
            className="bg-red-600 text-white py-4 text-center hover:bg-red-700 transition-colors duration-200"
            onClick={onLogout}
          >
            Logout
          </button>
        </div>
      </footer>
    </div>
  );
};

export default RewardsPage;