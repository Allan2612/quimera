// Recorridos de punta a punta en el navegador: lo que una persona hace en el sitio.
import { readFileSync } from 'node:fs';
import { expect, test, type Page } from '@playwright/test';

/** Reemplaza el código del editor como si se escribiera */
async function type(page: Page, code: string) {
  await page.locator('.cm-content').click();
  await page.keyboard.press('ControlOrMeta+A');
  await page.keyboard.press('Delete');
  await page.keyboard.insertText(code);
}

test('ejecuta un ejemplo y muestra la salida', async ({ page }) => {
  await page.goto('/');
  await page.getByRole('button', { name: /Ejecutar/ }).click();
  await expect(page.locator('#status')).toHaveText(/Listo/);
  await expect(page.locator('#output')).toContainText('Hola, mundo');
});

test('un error se marca y enlaza a su explicación', async ({ page }) => {
  await page.goto('/');
  await type(page, 'Integer total = 10;\nPrint("total: " + total);\n');
  await page.getByRole('button', { name: /Ejecutar/ }).click();
  await expect(page.locator('#status')).toHaveText('1 error');
  await expect(page.locator('.cm-lintRange-error')).toHaveCount(1);
  await page.getByRole('link', { name: '¿Qué significa?' }).click();
  await expect(page).toHaveURL(/\/docs\/errores\/#e3000$/);
  await expect(page.locator('#e3000')).toBeVisible();
});

test('un ciclo infinito se detiene', async ({ page }) => {
  await page.goto('/');
  await type(page, 'While (True)\nEnd\n');
  await page.getByRole('button', { name: /Ejecutar/ }).click();
  await expect(page.locator('#status')).toHaveText('Detenido', { timeout: 15_000 });
});

test('un reto se resuelve con su solución y queda guardado', async ({ page }) => {
  const solution = readFileSync(new URL('../retos-soluciones/suma.qmr', import.meta.url), 'utf8');
  await page.goto('/retos/#suma');
  await page.getByRole('button', { name: 'Revisar' }).click();
  await expect(page.locator('#verdict')).toHaveText(/Pasan 0 de 4/);
  await type(page, solution);
  await page.getByRole('button', { name: 'Revisar' }).click();
  await expect(page.locator('#verdict')).toHaveText(/Reto resuelto/);
  await page.reload();
  await expect(page.locator('a[href="#suma"] .done')).toBeVisible();
});

test('la documentación abre sus ejemplos en el editor', async ({ page }) => {
  await page.goto('/docs/referencia/');
  await page.getByRole('link', { name: 'Abrir en el editor' }).first().click();
  await expect(page).toHaveURL(/\/#p=/);
  await page.getByRole('button', { name: /Ejecutar/ }).click();
  await expect(page.locator('#status')).toHaveText(/Listo/);
});

test('el tema elegido se mantiene al recargar', async ({ page }) => {
  await page.emulateMedia({ colorScheme: 'light' });
  await page.goto('/');
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'light');
  await page.getByRole('button', { name: 'Cambiar a tema oscuro' }).click();
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'dark');
  await expect(page.getByRole('button', { name: 'Cambiar a tema claro' })).toBeVisible();
  await page.goto('/docs/');
  await expect(page.locator('html')).toHaveAttribute('data-theme', 'dark');
});

test.describe('en el celular', () => {
  test.use({ viewport: { width: 390, height: 844 } });

  test('Editor y Salida van en pestañas', async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('.pane-output')).toBeHidden();
    await page.getByRole('button', { name: /Ejecutar/ }).click();
    await expect(page.locator('.pane-output')).toBeVisible();
    await expect(page.locator('.pane-editor')).toBeHidden();
    await page.getByRole('tab', { name: 'Editor' }).click();
    await expect(page.locator('.pane-editor')).toBeVisible();
  });

  for (const path of ['/', '/aprende/', '/retos/', '/docs/errores/']) {
    test(`sin desborde horizontal en ${path}`, async ({ page }) => {
      await page.goto(path);
      expect(await page.evaluate(() => document.documentElement.scrollWidth - innerWidth)).toBeLessThanOrEqual(0);
    });
  }
});
