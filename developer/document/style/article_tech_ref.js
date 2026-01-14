/*
  Article Layout: Technical Reference
  Standard: Theme 1.0
  Description: High-readability layout for technical documentation on screens.
  Features: Sans-serif, justified text, distinct headers, boxed code.
*/
(function(){
  const RT = window.StyleRT = window.StyleRT || {};

  RT.article = function() {
    RT.config = RT.config || {};
    
    // Default Configuration
    RT.config.article = {
       font_family: '"Noto Sans", "Segoe UI", "Helvetica Neue", sans-serif'
      ,line_height: "1.8"       // Generous spacing for screen reading
      ,font_size: "18px"        // Large base size for clarity
      ,max_width: "820px" 
      ,margin: "0 auto"
    };

    const conf = RT.config.article;
    const article_seq = document.querySelectorAll("RT-article");

    // HURDLE
    if(RT.debug && RT.debug.log) RT.debug.log('selector', `RT.article found ${article_seq.length} elements.`);
    if(article_seq.length === 0) return;

    // 1. Apply Container Styles
    article_seq.forEach( (article) =>{
      const style = article.style;
      style.display = "block";
      style.fontFamily = conf.font_family;
      style.fontSize = conf.font_size;
      style.lineHeight = conf.line_height;
      style.maxWidth = conf.max_width;
      style.margin = conf.margin;
      style.padding = "0 20px"; // Mobile buffer
      
      // Default text color from Theme 1.0
      style.color = "var(--rt-content-main)";
    });

    // 2. Inject Child Typography
    const style_id = 'rt-article-typography';
    if (!document.getElementById(style_id)) {
      const style_el = document.createElement('style');
      style_el.id = style_id;
      
      style_el.textContent = `
        /* --- HEADERS --- */
        rt-article h1 { 
          color: var(--rt-brand-primary);
          font-size: 2.4em; 
          font-weight: 700; 
          margin-top: 1.2em; 
          margin-bottom: 0.6em; 
          border-bottom: 2px solid var(--rt-brand-primary);
          padding-bottom: 0.3em;
          line-height: 1.2;
          letter-spacing: -0.02em;
        }
        
        rt-article h2 { 
          color: var(--rt-brand-secondary);
          font-size: 1.8em; 
          font-weight: 600; 
          margin-top: 1.5em; 
          margin-bottom: 0.5em; 
          border-bottom: 1px dotted var(--rt-border-default);
        }

        rt-article h3 { 
          color: var(--rt-brand-tertiary);
          font-size: 1.4em; 
          font-weight: 600;
          margin-top: 1.4em; 
          margin-bottom: 0.5em;
          font-style: italic;
        }
        
        rt-article h4, rt-article h5, rt-article h6 {
           color: var(--rt-content-main);
           font-weight: bold;
           margin-top: 1.2em;
        }

        /* --- BODY TEXT --- */
        rt-article p { 
          margin-bottom: 1.4em; 
          text-align: justify; 
          hyphens: auto;
          color: var(--rt-content-main);
        }

        /* --- RICH ELEMENTS --- */
        rt-article blockquote { 
          border-left: 4px solid var(--rt-brand-secondary); 
          margin: 1.5em 0; 
          padding: 0.5em 1em; 
          font-style: italic; 
          color: var(--rt-content-muted);
          background: var(--rt-surface-1);
          border-radius: 0 4px 4px 0;
        }

        rt-article ul, rt-article ol {
          margin-bottom: 1.4em;
          padding-left: 2em;
        }
        rt-article li {
           margin-bottom: 0.4em;
        }
        rt-article li::marker {
          color: var(--rt-brand-secondary);
          font-weight: bold;
        }
        
        /* Links */
        rt-article a {
          color: var(--rt-brand-link);
          text-decoration: none;
          border-bottom: 1px dotted var(--rt-border-default);
          transition: all 0.2s;
        }
        rt-article a:hover {
          color: var(--rt-brand-primary);
          border-bottom: 1px solid var(--rt-brand-primary);
          background: var(--rt-surface-1);
        }
        
        /* --- CODE & TECHNICAL --- */
        /* Inline Code */
        rt-article code {
          background-color: var(--rt-surface-code);
          color: var(--rt-syntax-keyword);
          padding: 0.2em 0.4em;
          border-radius: 4px;
          font-family: "Consolas", "Monaco", monospace;
          font-size: 0.9em;
          border: 1px solid var(--rt-border-faint);
        }
        
        /* Preformatted Blocks (if not handled by RT_code.js) */
        rt-article pre {
           background: var(--rt-surface-code);
           padding: 1em;
           border-radius: 4px;
           overflow-x: auto;
           border: 1px solid var(--rt-border-default);
        }
      `;
      document.head.appendChild(style_el);
    }
  };
})();

