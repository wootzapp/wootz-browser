import React, { useState, useEffect } from 'react';
import ExtensionList from './components/ExtensionList';
import BrowserBridge from './components/browserbridge';
import extensionsData from './components/extensionsData';

const CACHE_DURATION = 1000 * 60 * 60; // 1 hour
const CACHE_KEY = 'extensions_cache';

function App() {
  const [extensions, setExtensions] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const browserBridge = BrowserBridge.getInstance();

  if (window.location.href.includes('wootzapp://extension-store')) {
    document.body.style.overscrollBehavior = 'none';
  }

  useEffect(() => {
    setLoading(false);
    fetchExtensions();
  }, []);

  const fetchExtensions = async () => {
    try {
      // Check cache first
      const cached = localStorage.getItem(CACHE_KEY);
      if (cached) {
        const { data, timestamp } = JSON.parse(cached);
        if (Date.now() - timestamp < CACHE_DURATION) {
          setExtensions(data);
          return;
        }
      }

      // Fetch extensions from the C++ backend
      const data = await browserBridge.fetchExtensions();
      
      if (!data || !data.extensions) {
        throw new Error('Invalid data format');
      }

      // Update cache
      localStorage.setItem(CACHE_KEY, JSON.stringify({
        data: data.extensions,
        timestamp: Date.now(),
      }));

      setExtensions(data.extensions);
      setError(null);
    } catch (error) {
      console.error('Error fetching extensions:', error);
      setError(error.message);
      setExtensions([]);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="w-full min-h-screen bg-white">
      <header className="sticky top-0 bg-white shadow-sm z-10 p-4">
        <h1 className="text-xl font-bold text-gray-800 text-center">Extension Store</h1>
      </header>
      <main className="p-4">
        {loading ? (
          <div className="text-center p-5 text-gray-600">Loading...</div>
        ) : error ? (
          <div className="text-center p-5 text-red-600">
            Error: {error}
            <button 
              onClick={fetchExtensions}
              className="block w-full mt-2 px-4 py-2 bg-blue-500 text-white rounded hover:bg-blue-600"
            >
              Retry
            </button>
          </div>
        ) : extensionsData.extensions.length === 0 ? (
          <div className="text-center p-5 text-gray-600">No extensions found</div>
        ) : (
          <ExtensionList extensions={extensionsData.extensions} />
        )}
      </main>
    </div>
  );
}

export default App;