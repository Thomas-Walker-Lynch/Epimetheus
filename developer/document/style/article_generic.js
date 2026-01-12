/*
  Sets basic document theme colors.
  No layout enforcement (leaves flow to default HTML block behavior).
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.article_generic = function() {
  const RT = window.StyleRT;
  const theme = RT.active_theme ? RT.active_theme() : {};

  const body = document.body;
  
  // Basic Theme Colors (Required for RT_code physics)
  body.style.backgroundColor = theme.background || '#000';
  body.style.color = theme.foreground || '#ddd';
  body.style.fontFamily = '"Noto Sans JP", sans-serif';
  
  // Reset margins to prevent browser defaults from interfering with your page.js
  body.style.margin = '0';
  body.style.padding = '0';
};
