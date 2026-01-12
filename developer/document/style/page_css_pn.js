/*
  Defines the appearance of the <RT-PAGE> container.
  Includes a page counter in the bottom-right and typographic constraints.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.page = function() {
  const RT = window.StyleRT;
  const style_id = 'rt-page-styles';
  const fixed_height = RT.page_height ? `${RT.page_height}px` : '1056px';

  if (!document.getElementById(style_id)) {
    const style_el = document.createElement('style');
    style_el.id = style_id;
    
    // We use CSS counters to automatically number the pages
    style_el.textContent = `
      body {
        counter-reset: rt-page-counter;
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
        height: ${fixed_height};
        position: relative;
        filter: drop-shadow(8px 12px 40px hsl(44, 96%, 47%));
        counter-increment: rt-page-counter;
      }

      /* Typographic Constraints */
      rt-page p {
        orphans: ${RT.orphans || 4};
        widows: ${RT.widows || 4};
      }

      /* The Page Counter */
      rt-page::after {
        content: "Page " counter(rt-page-counter);
        position: absolute;
        bottom: 1.5rem;
        right: 3rem;
        font-family: sans-serif;
        font-size: 0.8rem;
        color: hsl(42, 100%, 20%); /* Faded gold */
      }
    `;
    document.head.appendChild(style_el);
  }

  if (RT.debug) RT.debug.log('style', `Pages numbered and fixed at ${fixed_height}.`);
};
