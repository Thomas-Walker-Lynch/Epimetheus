/*
  Defines the appearance of a "Page" container.
  Restores the Original "Gold Glow" Scheme.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.page = function() {
  const RT = window.StyleRT;
  
  // Fetch Theme
  const theme = RT.active_theme ? RT.active_theme() : {};
  // Use the bright accent for the glow
  const glowColor = theme.accent || 'gold'; 

  document.querySelectorAll('.page').forEach(el => {
    // A. Dimensions (The "Web" Look)
    el.style.width = '1200px'; 
    el.style.maxWidth = '95vw'; // Responsive safety
    el.style.height = 'auto';   // Hug content
    el.style.minHeight = '200px'; // Minimum sanity check
    el.style.boxSizing = 'border-box';
    
    // B. The "Original" Visuals
    // 1. Background: Likely darker/transparent to let the theme breathe
    el.style.backgroundColor = 'rgba(0, 0, 0, 0.5)'; 
    
    // 2. Border: Thin accent line
    el.style.border = `1px solid ${glowColor}`;
    
    // 3. Shadow: The "Gold Drop Shadow" you missed
    // Format: offset-x | offset-y | blur-radius | color
    el.style.boxShadow = `0 0 15px ${glowColor}`; 
    
    el.style.margin = '0 auto'; 
    
    // C. Typography Base
    el.style.padding = '2rem 4rem'; // Standard comfortable web reading padding
    el.style.lineHeight = '1.6';
  });
};
