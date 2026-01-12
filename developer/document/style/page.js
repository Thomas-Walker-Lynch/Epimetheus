/*
  Defines the appearance of the <RT-PAGE> container via a CSS block.
  Uses high-contrast offsets to ensure the drop shadow is visible.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.page = function() {
  const RT = window.StyleRT;
  const style_id = 'rt-page-styles';
  
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
        
        /* Increased vertical margin to prevent shadow clipping */
        margin: 4rem auto 6rem auto; 
        padding: 3rem;
        box-sizing: border-box;
        background-color: hsl(0, 0%, 0%);
        border: 1px solid hsl(42, 100%, 50%);
        
        /* 1. The Offset Drop Shadow (Bottom-Right)
           2. The Ambient Glow (Centered)
        */
        box-shadow: 15px 15px 30px rgba(0, 0, 0, 0.9), 
                    0 0 15px hsl(42, 100%, 15%);
        
        height: auto;
        
        /* Ensure the shadow isn't cut off by the container's edges */
        overflow: visible; 
      }
    `;
    document.head.appendChild(style_el);
  }

  if (RT.debug) RT.debug.log('style', 'CSS block updated with 15px shadow offset.');
};
