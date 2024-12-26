import React, { useEffect,useState } from 'react';
import { useNavigate } from 'react-router-dom';
import BrowserBridge from './BrowserBridge';
import art1 from '../images/art1.png'

const Toast = ({ message, onClose }) => (
  <div className="fixed inset-0 flex items-center justify-center z-50 bg-black bg-opacity-50 backdrop-blur-sm" onClick={onClose}>
    <div className="bg-yellow-100 border-l-4 border-yellow-500 text-yellow-700 p-4 rounded-lg shadow-xl max-w-sm w-full mx-4">
      <div className="flex items-center">
        <div className="py-1">
          <svg className="fill-current h-6 w-6 text-yellow-500 mr-4" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20">
            <path d="M2.93 17.07A10 10 0 1 1 17.07 2.93 10 10 0 0 1 2.93 17.07zm12.73-1.41A8 8 0 1 0 4.34 4.34a8 8 0 0 0 11.32 11.32zM9 11V9h2v6H9v-4zm0-6h2v2H9V5z"/>
          </svg>
        </div>
        <p className="font-bold text-center flex-1">{message}</p>
      </div>
    </div>
  </div>
);

const LoginPage = ({ onLoginSuccess }) => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  const navigate = useNavigate();
  const browserBridge = BrowserBridge.getInstance();

  useEffect(() => {
    if (error) {
      const timer = setTimeout(() => {
        setError('');
      }, 1500);
      return () => clearTimeout(timer);
    }
  }, [error]);
  
  const handleSubmit = async (e) => {
    e.preventDefault();
    setError('');
    setLoading(true);
  
    try {
      const response = await browserBridge.loginWallet(email, password);
      if (response && response.id_token) {
        localStorage.setItem('authToken', response.id_token);
        onLoginSuccess(response.id_token);
        navigate('/rewards');
      } else {
        throw new Error('Invalid response from server');
      }
    } catch (error) {
      console.error('Login Error:', error);
      console.log('Exact error message:', error.message);
      
      // Now we can directly use the error message
      if (error.message === 'Invalid Email or Password') {
        setError('Invalid Email or Password\nPlease try again');
      } else {
        setError('Login failed. Please try again.');
      }
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-white p-4 sm:p-6">
      {error && <Toast message={error} onClose={() => setError('')} />}
      <div className="w-full max-w-sm sm:max-w-md">
        <img src={art1} alt="Artifact Logo" className="mx-auto h-10 sm:h-12 mb-6 sm:mb-8" />
        <h2 className="text-xl sm:text-2xl font-bold mb-4 sm:mb-6 text-center">Login</h2>
        <form onSubmit={handleSubmit} className="space-y-4 sm:space-y-6">
          {/* Email input */}
          <div>
            <label htmlFor="email" className="block mb-1 sm:mb-2 text-sm font-medium text-gray-700">Email</label>
            <input
              type="email" id="email" placeholder="Enter Email"
              className="w-full px-3 py-2 text-sm sm:text-base border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              value={email} onChange={(e) => setEmail(e.target.value)} required
            />
          </div>
          {/* Password input */}
          <div>
            <label htmlFor="password" className="block mb-1 sm:mb-2 text-sm font-medium text-gray-700">Password</label>
            <input
              type="password" id="password" placeholder="Enter Password"
              className="w-full px-3 py-2 text-sm sm:text-base border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              value={password} onChange={(e) => setPassword(e.target.value)} required
            />
          </div>
          <button
            type="submit"
            className="w-full bg-black text-white py-2 px-4 text-sm sm:text-base rounded-md hover:bg-gray-800 focus:outline-none focus:ring-2 focus:ring-gray-500 focus:ring-offset-2"
            disabled={loading}
          >
            {loading ? 'Logging in...' : 'Submit'}
          </button>
        </form>
      </div>
    </div>
  );
};

export default LoginPage;