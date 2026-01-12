/*
  Defines the appearance of the <RT-PAGE> container.
  Enforces a fixed height to create a uniform book-like appearance.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.page = function() {
  const RT = window.StyleRT;
  const style_id = 'rt-page-styles';
  
  // Use the established page height or a standard 11-inch default
  const fixed_height = RT.page_height ? `${RT.page_height}px` : '1056px';

  if (!document.getElementById(style_id)) {
    const style_el = document.createElement('style');
    style_el.id = style_id;
    style_el.textContent = `
      html, body {
        background-color: #1a1a1a !important; 
        margin: 0;
        padding: 0;
      }

      rt-page {
        display: block;
        max-width: 50rem;
        width: 100%;
        margin: 4rem auto 6rem auto;
        padding: 3rem;
        box-sizing: border-box;
        background-color: hsl(0, 0%, 0%);
        border: 1px solid hsl(42, 100%, 50%);
        
        /* Fixed Height Enforcement */
        height: ${fixed_height};
        overflow: visible;

        /* The Gold Leaf Glow */
        filter: drop-shadow(8px 12px 40px hsl(44, 96%, 47%));
      }
    `;
    document.head.appendChild(style_el);
  }

  if (RT.debug) RT.debug.log('style', `Fixed height of ${fixed_height} applied to all pages.`);
};
