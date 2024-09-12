import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import backImage from '../images/back.svg'
import wootzImage from '../images/wootz.png';
import userDefaultImage from '../images/user.png';
import BrowserBridge from './BrowserBridge';

const ProfileField = ({ label, value }) => {
  if (!value) return null;
  return (
    <div className="mb-4 bg-gray-100 p-3 rounded-lg">
      <dt className="text-sm font-medium text-gray-500">{label}</dt>
      <dd className="mt-1 text-sm text-gray-900 break-all">{value}</dd>
    </div>
  );
};

const AffiliateReferral = ({ referral }) => (
  <li className="mb-2">
    {referral.reference_id && <div><span className="font-medium">Reference ID:</span> {referral.reference_id}</div>}
    {referral.reference_type && <div><span className="font-medium">Type:</span> {referral.reference_type}</div>}
    {referral.reference_detail && <div><span className="font-medium">Detail:</span> {JSON.stringify(referral.reference_detail)}</div>}
  </li>
);

const LoadingScreen = () => {
  const [dots, setDots] = useState('');

  useEffect(() => {
    const interval = setInterval(() => {
      setDots(prevDots => (prevDots.length >= 3 ? '' : prevDots + '.'));
    }, 150);

    return () => clearInterval(interval);
  }, []);

  return (
    <div className="fixed inset-0 z-50 flex flex-col items-center justify-center">
      <div 
        className="absolute inset-0 bg-gray-100 opacity-75 backdrop-filter backdrop-blur-sm"
      ></div>
      <div 
        className="z-10 w-32 h-32 "
        style={{
          backgroundImage: `url(${wootzImage})`,
          backgroundPosition: 'center',
          backgroundRepeat: 'no-repeat',
          backgroundSize: 'contain'
        }}
      ></div>
      <div className="z-10 text-2xl font-bold text-gray-800">
        Loading{dots}
      </div>
    </div>
  );
};

const ProfilePage = () => {
  const [profile, setProfile] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const navigate = useNavigate();
  const browserBridge = BrowserBridge.getInstance();

  useEffect(() => {
    const fetchProfile = async () => {
      try {
        const token = localStorage.getItem('authToken');
        const data = await browserBridge.getUserProfile(token);
        if (data && Object.keys(data).length > 0) {
          setProfile(data);
        } else {
          setError('Received empty profile data');
        }
      } catch (err) {
        console.error('Error fetching profile:', err);
        setError(`Failed to load profile: ${err.message}`);
      } finally {
        setLoading(false);
      }
    };

    fetchProfile();
  }, []);

  console.log('Current state:', { loading, error, profile });

  if (loading) return <LoadingScreen />;
  
  if (error) return <div className="text-center mt-8 text-red-500">{error}</div>;
  
  if (!profile) {
    console.log('Profile is null, rendering fallback');
    return <div className="text-center mt-8">No profile data available</div>;
  }

  return (
    <div className="min-h-screen bg-gray-100 pt-16">
      <header className="bg-white shadow-sm fixed top-0 left-0 right-0 z-10">
        <div className="max-w-7xl mx-auto py-4 px-4 sm:px-6 lg:px-8 flex justify-between items-center">
          <button className="w-6 h-6" onClick={() => navigate(-1)}>
            <img src={backImage} alt="Back" className="w-full h-full" />
          </button>
          <h1 className="text-xl font-semibold">Profile</h1>
          <div className="w-6"></div>
        </div>
      </header>

      <main className="max-w-md mx-auto">
        <div className="bg-white shadow overflow-hidden sm:rounded-lg">
          <div className="px-4 py-5 sm:px-6 flex flex-col items-center profile-header">
            <div 
              className="w-24 h-24 bg-gray-300 rounded-full mb-4 profile-circle"
              style={{ backgroundImage: `url(${profile.avatar_uri || userDefaultImage})`, backgroundSize: 'cover', backgroundPosition: 'center' }}
            ></div>
            <h2 id="profileName" className="text-xl font-bold">{profile.display_name || 'N/A'}</h2>
            <p id="profileUsername" className="text-gray-500">@{profile.username || 'N/A'}</p>
          </div>
          <div className="border-t border-gray-200 px-4 py-5">
            <ProfileField label="Email" value={profile.email} />
            <ProfileField label="Role" value={profile.role} />
            <ProfileField label="Location" value={profile.location} />
            <ProfileField label="Biography" value={profile.biography} />
            <ProfileField label="Created At" value={profile.created_at} />
            <ProfileField label="Omnikey ID" value={profile.omnikey_id} />
            
            {profile.affiliate_referral && profile.affiliate_referral.length > 0 && (
              <div className="mb-4 bg-gray-100 p-3 rounded-lg">
                <dt className="text-sm font-medium text-gray-500">Affiliate Referrals</dt>
                <dd className="mt-1 text-sm text-gray-900">
                  <ul>
                    {profile.affiliate_referral.map((referral, index) => (
                      <AffiliateReferral key={index} referral={referral} />
                    ))}
                  </ul>
                </dd>
              </div>
            )}
          </div>
        </div>
      </main>
    </div>
  );
};

export default ProfilePage;