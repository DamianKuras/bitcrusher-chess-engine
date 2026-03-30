/** @type {import('tailwindcss').Config} */
export default {
  content: ['./index.html', './src/**/*.{js,ts,jsx,tsx}'],
  theme: {
    extend: {
      colors: {
        // RGB vars so opacity modifiers like bg-surface/80 work
        bg:        'rgb(var(--bg)        / <alpha-value>)',
        surface:   'rgb(var(--surface)   / <alpha-value>)',
        line:      'rgb(var(--line)      / <alpha-value>)',
        copy:      'rgb(var(--copy)      / <alpha-value>)',
        muted:     'rgb(var(--muted)     / <alpha-value>)',
        accent:    'rgb(var(--accent)    / <alpha-value>)',
        'accent-h':'rgb(var(--accent-h)  / <alpha-value>)',
        danger:    'rgb(var(--danger)    / <alpha-value>)',
      },
      fontFamily: {
        sans: ['Inter', 'system-ui', 'sans-serif'],
        mono: ['"Fira Code"', '"Cascadia Code"', 'monospace'],
      },
    },
  },
  plugins: [],
}
