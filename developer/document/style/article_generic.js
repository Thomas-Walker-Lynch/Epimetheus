/*
  Sets basic document theme colors and dimensions.
  Sets width immediately so layout-based pagination is accurate.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.article_generic = function() {
  const RT = window.StyleRT;
  const theme = RT.active_theme ? RT.active_theme() : {};

  const body = document.body;
  
  // 1. Basic Theme Colors
  body.style.backgroundColor = theme.background || 'hsl(0, 0%, 0%)';
  body.style.color = theme.foreground || 'hsl(42, 100%, 80%)';
  body.style.fontFamily = '"Noto Sans JP", sans-serif';
  
  // 2. Global Flow Reset
  body.style.display = 'block'; 
  body.style.margin = '0';
  body.style.padding = '0';

  // 3. Dimensions
  // We apply the width to the body now so that getBoundingClientRect() 
  // in the paginator reflects the final text wrapping.
  body.style.maxWidth = '50rem';
  body.style.margin = '0 auto'; 
  
  // 4. Centered Headers
  // We apply this via a quick style injection or direct selection
  const h1s = document.querySelectorAll('h1');
  h1s.forEach(h => {
    h.style.textAlign = 'center';
    h.style.width = '100%';
  });

  // 5. Layout Variables
  // We store the target page height for the paginator to find.
  RT.page_height = 1056; 

  // Typographic constraints for future line-splitting logic
  RT.orphans = 4;
  RT.widows = 4;
  
  if (RT.debug) RT.debug.log('style', 'Article generic setup: H1 centered, orphans/widows set to 4.');
};
