/*
  Defines the appearance of the <RT-PAGE> container.
  Uses grouped settings from RT.layout and RT.typography.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.page = function() {
  const RT = window.StyleRT;
  const style_id = 'rt-page-styles';
  
  // Safety fallbacks in case article_generic failed or hasn't run
  const layout = RT.layout || { page_height: 1056, page_margin: '4rem auto' };
  const typo = RT.typography || { orphans: 4, widows: 4 };

  if (!document.getElementById(style_id)) {
    const style_el = document.createElement('style');
    style_el.id = style_id;
    
    style_el.textContent = `
      body {
        counter-reset: rt-page-counter;
      }

      rt-page {
        display: block;
        max-width: ${layout.page_width || '50rem'};
        width: 100%;
        margin: ${layout.page_margin};
        padding: ${layout.page_padding || '3rem'};
        box-sizing: border-box;
        background-color: hsl(0, 0%, 0%);
        border: 1px solid hsl(42, 100%, 50%);
        height: ${layout.page_height}px;
        position: relative;
        filter: drop-shadow(8px 12px 40px hsl(44, 96%, 47%));
        counter-increment: rt-page-counter;
      }

      rt-page p {
        orphans: ${typo.orphans};
        widows: ${typo.widows};
      }

      /* Brightened Page Counter */
      rt-page::after {
        content: "Page " counter(rt-page-counter);
        position: absolute;
        bottom: 1.5rem;
        right: 3rem;
        font-family: "Noto Sans JP", sans-serif;
        font-size: 0.9rem;
        font-weight: bold;
        color: hsl(42, 100%, 75%); /* Much brighter, high-contrast gold */
      }
    `;
    document.head.appendChild(style_el);
  }
};
