import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { resolve } from 'path';

export default defineConfig({
  plugins: [react()],
  build: {
    outDir: 'dist',
    rollupOptions: {
      input: {
        main: 'index.html'
      }
    },
    copyPublicDir: true
  },
  server: {
    port: 3000
  },
  publicDir: 'public'
}); 