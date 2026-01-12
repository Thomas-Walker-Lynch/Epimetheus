/*
  Sets basic document theme colors and dimensions.
  Groups parameters into semantic dictionaries (layout, typography).
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.article_generic = function() {
  const RT = window.StyleRT;
  const theme = RT.active_theme ? RT.active_theme() : {};

  // 1. Theme and Global Flow
  const body = document.body;
  body.style.backgroundColor = theme.background || 'hsl(0, 0%, 0%)';
  body.style.color = theme.foreground || 'hsl(42, 100%, 80%)';
  body.style.fontFamily = '"Noto Sans JP", sans-serif';
  body.style.display = 'block'; 
  body.style.margin = '0';
  body.style.padding = '0';

  // 2. Settings: Layout Group
  RT.layout = {
    page_height: 1056,
    page_width: '50rem',
    page_margin: '4rem auto 6rem auto',
    page_padding: '3rem'
  };

  // 3. Settings: Typography Group
  RT.typography = {
    orphans: 4,
    widows: 4,
    h1_align: 'center'
  };

  // 4. Apply Dimensions and Alignment
  body.style.maxWidth = RT.layout.page_width;
  body.style.margin = '0 auto'; 

  const h1s = document.querySelectorAll('h1');
  h1s.forEach(h => {
    h.style.textAlign = RT.typography.h1_align;
    h.style.width = '100%';
  });
};
