/*
  Provides the color palette for the RT theme.
  Registers itself as the active system theme upon load.
*/
function theme_RT(){
  return {
    name: 'theme_RT', // Identity
    
    // Palette
    background: 'hsl(0, 0%, 0%)',
    foreground: 'hsl(42, 100%, 80%)',
    accent: 'hsl(42, 100%, 50%)',
    faded: 'hsl(42, 100%, 20%)',
    highlight: 'hsl(42, 100%, 90%)'
  };
}

window.StyleRT = window.StyleRT || {};
window.StyleRT.theme_RT = theme_RT;

// Generic Interface: Set this as the current active theme
window.StyleRT.active_theme = theme_RT;
