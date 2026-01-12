/*
  Defines the appearance of the <RT-PAGE> container.
  Uses the high-performance 'filter: drop-shadow' discovered in testing.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.page = function() {
  const RT = window.StyleRT;
  const style_id = 'rt-page-styles';
  
  if (!document.getElementById(style_id)) {
    const style_el = document.createElement('style');
    style_el.id = style_id;
    style_el.textContent = `
      rt-page {
        display: block;
        max-width: 50rem;
        width: 100%;
        margin: 4rem auto 6rem auto;
        padding: 3rem;
        box-sizing: border-box;
        background-color: hsl(0, 0%, 0%);
        border: 1px solid hsl(42, 100%, 50%);
        
        /* Using the discovered Gold Leaf HSL */
        filter: drop-shadow(8px 12px 40px hsl(40, 96%, 47%));
        
        height: auto;
      }
    `;
    document.head.appendChild(style_el);
  }

  if (RT.debug) RT.debug.log('style', 'CSS block updated with Gold Leaf drop-shadow filter.');
};
