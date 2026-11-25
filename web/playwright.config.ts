import { defineConfig, devices } from '@playwright/test';

// Pruebas de punta a punta contra el sitio ya compilado (npm run build) servido con vite preview.
export default defineConfig({
  testDir: 'e2e',
  fullyParallel: true,
  retries: process.env.CI ? 1 : 0,
  reporter: process.env.CI ? 'github' : 'list',
  // El intérprete en WebAssembly tarda más cuando corren varios navegadores a la vez
  expect: { timeout: 15_000 },
  use: { baseURL: 'http://localhost:4174', trace: 'retain-on-failure' },
  // En el CI basta Chromium; ALL_BROWSERS=1 suma Firefox y WebKit para revisar en local
  projects: [
    { name: 'chromium', use: { ...devices['Desktop Chrome'] } },
    ...(process.env.ALL_BROWSERS
      ? [
          { name: 'firefox', use: { ...devices['Desktop Firefox'] } },
          { name: 'webkit', use: { ...devices['Desktop Safari'] } },
        ]
      : []),
  ],
  webServer: {
    command: 'npx vite preview --port 4174 --strictPort',
    url: 'http://localhost:4174',
    reuseExistingServer: !process.env.CI,
  },
});
