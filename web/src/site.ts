// Comportamiento común a todas las páginas: tema claro/oscuro, color del código fijo,
// enlace activo del menú y la entrada suave de las secciones.
import { highlightToHtml } from './quimera-tokens';

/* ---------------- Tema ---------------- */

// Dos temas. Mientras la persona no elija, se sigue el del sistema; al tocar el botón se guarda su elección.
type Theme = 'claro' | 'oscuro';
const THEME_KEY = 'quimera.tema';
// El botón muestra a qué tema lleva: luna en el claro, sol en el oscuro
const ICON = {
  sol: '<circle cx="10" cy="10" r="3.5"/><path d="M10 2v2M10 16v2M2 10h2M16 10h2M4.3 4.3l1.4 1.4M14.3 14.3l1.4 1.4M4.3 15.7l1.4-1.4M14.3 5.7l1.4-1.4"/>',
  luna: '<path d="M16 12.5A7 7 0 0 1 7.5 4a7 7 0 1 0 8.5 8.5z"/>',
};

function readTheme(): Theme | null {
  try {
    const t = localStorage.getItem(THEME_KEY);
    return t === 'claro' || t === 'oscuro' ? t : null; // cualquier otro valor (o ninguno): el del sistema
  } catch {
    return null;
  }
}

const media = window.matchMedia('(prefers-color-scheme: dark)');
const isDark = (chosen: Theme | null) => chosen === 'oscuro' || (chosen === null && media.matches);

function applyTheme(chosen: Theme | null) {
  document.documentElement.dataset.theme = isDark(chosen) ? 'dark' : 'light';
}

export function initTheme() {
  let chosen = readTheme();
  applyTheme(chosen);
  const button = document.querySelector<HTMLButtonElement>('#theme');
  const paint = () => {
    if (!button) return;
    const dark = isDark(chosen);
    button.innerHTML = `<svg width="18" height="18" viewBox="0 0 20 20" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">${dark ? ICON.sol : ICON.luna}</svg>`;
    const label = dark ? 'Cambiar a tema claro' : 'Cambiar a tema oscuro';
    button.setAttribute('aria-label', label);
    button.title = label;
  };
  media.addEventListener('change', () => {
    applyTheme(chosen);
    paint();
  });
  paint();
  button?.addEventListener('click', () => {
    chosen = isDark(chosen) ? 'claro' : 'oscuro';
    try {
      localStorage.setItem(THEME_KEY, chosen);
    } catch {
      /* sin almacenamiento: dura hasta recargar */
    }
    applyTheme(chosen);
    paint();
  });
}

/* ---------------- Código fijo con colores ---------------- */

export function highlightStaticCode(root: ParentNode = document) {
  root.querySelectorAll<HTMLElement>('main code:not([data-hl])').forEach((el) => {
    if (el.closest('.cm-editor')) return;
    el.innerHTML = highlightToHtml(el.textContent ?? '');
    el.dataset.hl = '';
  });
}

/* ---------------- Menú: marcar la sección actual ---------------- */

function markCurrentNav() {
  const here = location.pathname.replace(/index\.html$/, '');
  document.querySelectorAll<HTMLAnchorElement>('.topbar nav a[href^="/"]').forEach((a) => {
    const path = new URL(a.href).pathname;
    if (path !== '/' && here.startsWith(path)) a.setAttribute('aria-current', 'page');
  });
}

/* ---------------- Entrada suave de las secciones ---------------- */

function revealOnScroll() {
  const io = new IntersectionObserver(
    (entries) => {
      for (const entry of entries) {
        if (entry.isIntersecting) {
          entry.target.classList.add('visible');
          io.unobserve(entry.target);
        }
      }
    },
    { rootMargin: '0px 0px -8% 0px' },
  );
  document.querySelectorAll('.reveal').forEach((el) => io.observe(el));
}

export function initSite() {
  initTheme();
  highlightStaticCode();
  markCurrentNav();
  revealOnScroll();
}
