import React, { useState, useEffect } from 'react';
import BrowserBridge from './browserbridge';

// Base64 placeholder icon (gray square with rounded corners)
const placeholderIcon = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAOxAAADsQBlSsOGwAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAALESURBVHic7d29bhNBFIbh9x87JCmCREFBRU1FSUGXa+Ay6LgWboBLSIGQkBAFBUJCQhQ4yf7M7O7ZmfO9z1NIopxk/M7ZmbXXa1kAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgAr2h7/4dPz8MHo/0fz6+f3X6H0AAADgf7Ef+eRvx8/Dc4L3N1/C9/R0ePz9Ofw5h8NTuKbG/mYTrrnb7cM1LXI9Z7QmQqsA1PjyI7UC0uL5S5/z8ebzxb97uv0Rrjk+3oVrIuF7CiLYQo0AHJJfZPSXHK2v9eVHawCUBKDkOVs8/1kYgNKdKglAi+cvDUCkRc2oANT48mv9AlsEoEQYgNLnrPH8JQEoec5WAaj15df6BbYIQIkwAKXP2eP5LQJQEoBaX36tX2DtAJQKA1D6nL2e3yIAJQGo9eXX+gXWDkCpMACtz3+tAFgEoNaXX+sXWDsApcIAtD7/tQJgEYBaX36tX2DtAJQKA9D6/NcKgEUAan35tX6BtQNQKgxA6/NfKwAWAaj15df6BdYOQKkwAK3Pf60AWASg1pdf6xdYOwClwgC0Pv+1AmARgFpffq1fYO0AlAoD0Pr81wqARQBqffm1foG1A1AqDEDr818rABYBqPXl1/oF1g5AqTAA0fkvDUBJjSwCYBGAWl9+rV9g7QCUCgMQnf/SAJTUyCIAFgGo9eXX+gXWDkCpMADR+S8NQEmNLAJgEYBaX36tX2DtAJQKAxCd/9IAlNTIIgAWAaj15df6BdYOQKkwANH5Lw1ASY0sAmARgFpffq1fYO0AlAoDEJ3/0gCU1MgiABYBqPXl1/oF1g5AqTAA0fkvDUBJjSwCYBGAWl9+rV9u7Q==";

function ExtensionCard({ extension }) {
  const [imageError, setImageError] = useState(false);
  const [isInstalled, setIsInstalled] = useState(false);

  // // Add useEffect to fetch extension info when component mounts
  // useEffect(() => {
  //   const bridge = BrowserBridge.getInstance();
  //   bridge.getExtensionInfo()
  //     .then(extensions => {
  //       console.log('Currently installed extensions:', extensions);
  //       // Check if current extension is in the installed list
  //       const isCurrentExtensionInstalled = extensions.some(
  //         ext => ext.id === extension.id
  //       );
  //       setIsInstalled(isCurrentExtensionInstalled);
  //     })
  //     .catch(error => {
  //       console.error('Failed to get extension info:', error);
  //     });
  // }, []); // Empty dependency array means this runs once on mount

  const handleImageError = () => {
    console.log('Image failed to load, using placeholder');
    setImageError(true);
  };

  const handleDownload = () => {
    console.log('Starting download process for:', extension.name);
    
    // Use chrome.downloads API if available, fallback to regular navigation
    if (chrome.downloads && chrome.downloads.download) {
      chrome.downloads.download({
        url: extension.download_url,
        filename: `${extension.name}.crx`,
        saveAs: false
      }, (downloadId) => {
        if (chrome.runtime.lastError) {
          console.error('Download failed:', chrome.runtime.lastError);
        } else {
          console.log('Download started with ID:', downloadId);
          setIsInstalled(true);
        }
      });
    } else {
      // Fallback to regular navigation
      window.location.href = extension.download_url;
      setIsInstalled(true);
    }
  };

  return (
    <div className="flex flex-row p-3 border border-gray-200 rounded-lg gap-3 hover:shadow-md transition-shadow w-full">
      <div className="w-16 h-16 flex-shrink-0 relative">
        <img 
          src={!imageError ? (extension.icon_url || placeholderIcon) : placeholderIcon}
          alt={extension.name} 
          className="w-full h-full object-cover rounded"
          onError={handleImageError}
        />
        {/* {!extension.icon_base64 && !imageError && (
          <div className="absolute inset-0 flex items-center justify-center bg-gray-100 rounded">
            <div className="animate-spin h-5 w-5 border-2 border-green-500 rounded-full border-t-transparent"></div>
          </div>
        )} */}
      </div>
      <div className="flex-1 min-w-0">
        <div className="flex justify-between items-start gap-2">
          <h2 className="text-lg font-semibold text-gray-800 truncate">
            {extension.name}
          </h2>
          <span className="text-sm text-gray-500 flex-shrink-0">v{extension.version}</span>
        </div>
        <p className="text-sm text-gray-600 line-clamp-2">{extension.description}</p>
        <div className="mt-2 flex items-center justify-between">
          <div className="flex flex-wrap gap-1">
            {extension.tags && extension.tags.map(tag => (
              <span key={tag} className="px-2 py-0.5 bg-gray-100 rounded-full text-xs text-gray-600">
                {tag}
              </span>
            ))}
          </div>
          <button
            onClick={handleDownload}
            disabled={isInstalled}
            className={`px-3 py-1 text-sm rounded transition-colors ${
              isInstalled 
                ? 'bg-green-500 text-white cursor-default'
                : 'bg-blue-500 text-white hover:bg-blue-600'
            }`}
          >
            {isInstalled ? 'Installed' : 'Install'}
          </button>
        </div>
      </div>
    </div>
  );
}

export default ExtensionCard;