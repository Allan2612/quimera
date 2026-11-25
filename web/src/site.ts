// Comportamiento común a todas las páginas: tema claro/oscuro, color del código fijo,
// enlace activo del menú y la entrada suave de las secciones.
import { highlightToHtml } from './quimera-tokens';

/* ---------------- Tema ---------------- */

type Theme = 'sistema' | 'claro' | 'oscuro';
const THEME_KEY = 'quimera.tema';
const ORDER: Theme[] = ['sistema', 'claro', 'oscuro'];
const LABEL: Record<Theme, string> = { sistema: 'Tema del sistema', claro: 'Tema claro', oscuro: 'Tema oscuro' };
const ICON: Record<Theme, string> = {
  sistema: '<rect x="2.5" y="3.5" width="15" height="10" rx="1.5"/><path d="M7 17h6M10 13.5V17"/>',
  claro: '<circle cx="10" cy="10" r="3.5"/><path d="M10 2v2M10 16v2M2 10h2M16 10h2M4.3 4.3l1.4 1.4M14.3 14.3l1.4 1.4M4.3 15.7l1.4-1.4M14.3 5.7l1.4-1.4"/>',
  oscuro: '<path d="M16 12.5A7 7 0 0 1 7.5 4a7 7 0 1 0 8.5 8.5z"/>',
};

function readTheme(): Theme {
  try {
    const t = localStorage.getItem(THEME_KEY) as Theme | null;
    return t && ORDER.includes(t) ? t : 'sistema';
  } catch {
    return 'sistema';
  }
}

const media = window.matchMedia('(prefers-color-scheme: dark)');

function applyTheme(theme: Theme) {
  const dark = theme === 'oscuro' || (theme === 'sistema' && media.matches);
  document.documentElement.dataset.theme = dark ? 'dark' : 'light';
}

export function initTheme() {
  let theme = readTheme();
  applyTheme(theme);
  media.addEventListener('change', () => applyTheme(theme));
  const button = document.querySelector<HTMLButtonElement>('#theme');
  if (!button) return;
  const paint = () => {
    button.innerHTML = `<svg width="18" height="18" viewBox="0 0 20 20" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">${ICON[theme]}</svg>`;
    button.setAttribute('aria-label', `${LABEL[theme]} (cambiar)`);
    button.title = LABEL[theme];
  };
  paint();
  button.addEventListener('click', () => {
    theme = ORDER[(ORDER.indexOf(theme) + 1) % ORDER.length];
    try {
      localStorage.setItem(THEME_KEY, theme);
    } catch {
      /* sin almacenamiento: dura hasta recargar */
    }
    applyTheme(theme);
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
