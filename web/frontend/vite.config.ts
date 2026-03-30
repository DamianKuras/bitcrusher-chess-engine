import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  server: {
    // Proxy API calls to FastAPI during development so the browser never
    // sees a cross-origin request (avoids CORS preflight for /search, etc.)
    proxy: {
      '/health':      'http://localhost:8000',
      '/legal-moves': 'http://localhost:8000',
      '/evaluate':    'http://localhost:8000',
      '/search':      'http://localhost:8000',
    },
  },
})
