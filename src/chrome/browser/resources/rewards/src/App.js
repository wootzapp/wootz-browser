import React, { useState, useEffect } from 'react';
import { BrowserRouter as Router, Route, Routes, Navigate } from 'react-router-dom';
import Login from './components/LoginPage';
import RewardsPage from './components/RewardsPage';
import ProfilePage from './components/ProfilePage';
import { ThirdwebProvider } from 'thirdweb/react';

function App() {
  const [isLoggedIn, setIsLoggedIn] = useState(false);

  useEffect(() => {
    const authToken = localStorage.getItem('authToken');
    if (authToken) {
      setIsLoggedIn(true);
    }
  }, []);

  const handleLoginSuccess = (token) => {
    setIsLoggedIn(true);
    localStorage.setItem('authToken', token);
  };

  const handleLogout = () => {
    setIsLoggedIn(false);
    localStorage.removeItem('authToken');
  };

  return (
    <ThirdwebProvider activeChain="ethereum">
      <Router>
        <div className="App">
          <Routes>
            <Route path="/login" element={
              isLoggedIn ? <Navigate to="/rewards" /> : <Login onLoginSuccess={handleLoginSuccess} />
            } />
            <Route path="/rewards" element={
              isLoggedIn ? <RewardsPage onLogout={handleLogout} /> : <Navigate to="/login" />
            } />
            <Route path="/profile" element={
              isLoggedIn ? <ProfilePage /> : <Navigate to="/login" />
            } />
            <Route path="*" element={<Navigate to={isLoggedIn ? "/rewards" : "/login"} />} />
          </Routes>
        </div>
      </Router>
    </ThirdwebProvider>
  );
}

export default App;