/*
  Generic Article Typography.
  Applies container settings and injects scoped CSS for standard HTML tags 
  (H1-H6, P, Blockquote) within the article.
*/
(function(){
  const RT = window.StyleRT = window.StyleRT || {};

  RT.article = function() {
    RT.config = RT.config || {};
    
    // Default Configuration
    RT.config.article = {
      font_family: '"Noto Sans JP", sans-serif'
      ,line_height: "1.75" // Increased slightly for readability
      ,font_size: "16px"
      ,max_width: "800px" 
      ,margin: "0 auto"
    };

    const conf = RT.config.article;
    const article_seq = document.querySelectorAll("RT-article");
    const theme = (RT.config.theme) ? RT.config.theme : { accent: 'gray', text: 'black' };

    // HURDLE
    if (RT.debug) RT.debug.log('selector', `RT.article found ${article_seq.length} elements.`);
    if(article_seq.length === 0) {
      if(RT.debug) RT.debug.error('selector', 'CRITICAL: No <RT-article> tags found.');
      return;
    }

    // 1. Apply Container Styles
    article_seq.forEach( (article) =>{
      const style = article.style;
      style.display = "block";
      style.fontFamily = conf.font_family;
      style.fontSize = conf.font_size;
      style.lineHeight = conf.line_height;
      style.maxWidth = conf.max_width;
      style.margin = conf.margin;
      style.backgroundColor = "transparent"; 
      
      // Ensure the accent color is available as a variable here too
      style.setProperty("--rt-accent", theme.accent);
    });

    // 2. Inject Child Typography (The missing piece)
    const style_id = 'rt-article-typography';
    if (!document.getElementById(style_id)) {
      const style_el = document.createElement('style');
      style_el.id = style_id;
      
      // We explicitly scope these to rt-article to avoid breaking the rest of the page
      style_el.textContent = `
        rt-article h1 { 
          font-size: 2.2em; 
          font-weight: 700; 
          margin-top: 1.5em; 
          margin-bottom: 0.5em; 
          border-bottom: 2px solid var(--rt-accent);
          padding-bottom: 0.2em;
          line-height: 1.2;
        }
        
        rt-article h2 { 
          font-size: 1.7em; 
          font-weight: 600; 
          margin-top: 1.5em; 
          margin-bottom: 0.5em; 
          color: var(--rt-accent);
          line-height: 1.3;
        }

        rt-article h3 { font-size: 1.4em; margin-top: 1.2em; margin-bottom: 0.5em; }
        
        rt-article p { 
          margin-bottom: 1.2em; 
          text-align: justify; 
          hyphens: auto;
        }

        rt-article blockquote { 
          border-left: 4px solid var(--rt-accent); 
          margin: 1.5em 0; 
          padding: 0.5em 0 0.5em 1.2em; 
          font-style: italic; 
          background: rgba(125,125,125, 0.05);
        }

        rt-article ul, rt-article ol {
          margin-bottom: 1.2em;
          padding-left: 2em;
        }
        
        rt-article li {
          margin-bottom: 0.3em;
        }
        
        /* Links inside articles */
        rt-article a {
          color: var(--rt-accent);
          text-decoration: none;
          border-bottom: 1px dotted var(--rt-accent);
        }
        rt-article a:hover {
          border-bottom: 1px solid var(--rt-accent);
        }
      `;
      document.head.appendChild(style_el);
    }
  };
})();
