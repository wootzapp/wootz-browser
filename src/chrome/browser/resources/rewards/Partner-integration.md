# Partner Integration Documentation

## Overview
This document outlines the process of integrating a partner application into the Chromium browser using a React-based frontend and C++ backend API calls to work in WebUIs.

## Frontend Development

### React Application
- Developed a responsive React application for the partner integration
- Ensured mobile compatibility
- Implemented API calls to the C++ backend

### Thirdweb Integration
- Added Thirdweb calls for wallet connection functionality
- Installed Thirdweb: `npm i thirdweb`
- Imported Thirdweb functions in relevant files

### BrowserBridge
- Implemented a BrowserBridge class to facilitate communication between the frontend and backend API calls

## Backend Development

### C++ API Calls
- Implemented API calls in C++ for improved security and performance
- Disabled TrustedTypesCSP policy in `rewards_ui.cc` to allow API calls

## Integration Process

### Build and Deployment
1. Built the React application using `npm run build`
2. Renamed the `build` directory to `dist`

### Resource Generation
1. Modified the script in `src/scripts/generate_grd.py` to include the new React application files
2. Generated a `.grd` file to include the dist directory contents by running `./generate_grd.py --module_name <--module--> --output <--grd_file-->`

### Build Configuration
- Generated `build.gn` files for the React directory

## File Structure
- `src/chrome/browser/resources/rewards`: Partner's page frontend
- `src/chrome/browser/resources/rewards/dist`: dist directory
- `src/chrome/browser/resources/rewards/src/components/LoginPage.js`: Login Page
- `src/chrome/browser/resources/rewards/src/components/BrowserBridge.js`: BrowserBridge implementation
- `src/chrome/browser/resources/rewards/src/components/RewardsPage.js`: Rewards page
- `src/chrome/browser/resources/rewards/src/components/ProfilePage.js`: User profile
- `src/chrome/browser/resources/rewards/BUILD.gn`: BUILD.gn for rewards
- `src/chrome/browser/ui/webui/rewards/rewards_ui.cc`: C++ backend implementation
- `src/chrome/browser/ui/webui/rewards/rewards_ui.h`: Header for rewards_ui.cc

### Compilation
- Ran `autoninja -C out/Default chrome_public_apk` to compile and integrate the application into the special WebUI