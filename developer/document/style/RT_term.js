/*
  Processes <RT-TERM> tags.
  Implements the "Definiendum Convention" (Italics + Theme Accent).
*/
function RT_term() {
  const RT = window.StyleRT;
  
  // Robust check for theme presence
  const theme = RT.active_theme ? RT.active_theme() : {};
  const accent = theme.accent || 'inherit';

  // Selects <RT-TERM>, <rt-term>, etc.
  document.querySelectorAll('rt-term').forEach(el => {
    el.style.fontStyle = 'italic';
    el.style.color = accent;
    el.style.paddingRight = '0.15em'; // Spacing for the italic slant
    el.style.display = 'inline';
  });
}

window.StyleRT = window.StyleRT || {};
window.StyleRT.RT_term = RT_term;
