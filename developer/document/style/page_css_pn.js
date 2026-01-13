/*
  Defines the appearance of the <RT-PAGE> container.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.page = function() {
  const RT = window.StyleRT;
  
  // Fallback accent
  const theme_accent = (RT.config && RT.config.theme) ? RT.config.theme.accent : "hsl(42, 100%, 50%)";

  RT.config = RT.config || {};
  
  // Fix: Define defaults independently
  const defaults = {
    width: "100%"
    ,height: "1056px"
    ,padding: "3rem"
    ,margin: "4rem auto"
    ,border_color: theme_accent
    ,shadow: "drop-shadow(8px 12px 40px hsl(44, 96%, 47%))"
  };

  // Fix: MERGE defaults into existing config. 
  // This allows overrides but prevents empty objects from causing "undefined" CSS.
  RT.config.page = Object.assign({}, defaults, RT.config.page || {});

  const conf = RT.config.page;
  const style_id = 'rt-page-styles';
  
  if (!document.getElementById(style_id)) {
    const style_el = document.createElement('style');
    style_el.id = style_id;
    
    style_el.textContent = `
      rt-article {
        counter-reset: rt-page-counter;
      }

      rt-page {
        display: block;
        width: ${conf.width};
        height: ${conf.height};
        margin: ${conf.margin};
        padding: ${conf.padding};
        box-sizing: border-box;
        
        background-color: black; 
        
        /* These should now be populated correctly */
        border: 1px solid ${conf.border_color};
        position: relative;
        filter: ${conf.shadow};
        
        counter-increment: rt-page-counter;
        overflow: hidden; 
      }

      rt-page::after {
        content: "Page " counter(rt-page-counter);
        position: absolute;
        bottom: 1.5rem;
        right: 3rem;
        font-family: "Noto Sans JP", sans-serif;
        font-size: 0.9rem;
        font-weight: bold;
        color: ${conf.border_color}; 
      }
    `;
    document.head.appendChild(style_el);
  }
};
