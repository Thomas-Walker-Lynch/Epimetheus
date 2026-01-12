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
  body.style.margin = '0 auto'; // Center the content area
  
  // We store the target page height on the RT object for the paginator to find.
  // This avoids assigning a height to the body itself (which would clip content).
  RT.page_height = 1056; 
};
