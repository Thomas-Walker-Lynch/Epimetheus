/*
  Master Loader & Orchestrator for StyleRT.
  Loads utility.js first to ensure infrastructure (RT.debug) exists.
*/

window.StyleRT = window.StyleRT || {};

window.StyleRT.do_style = function() {
  const RT = window.StyleRT;
  
  const modules = [
    'style/theme_RT.js',        
    'style/RT_term.js',         
    'style/RT_math.js',         
    'style/RT_code.js',         
    'style/article_generic.js', 
    'style/RT_TOC.js',          
    'style/paginate_by_element.js', 
    //    'style/page.js',
    'style/page_css.js',            
    'style/body_visibility_visible.js' 
  ];

  // 1. Bootloader: Get the utility/logger in place first
  const utility = document.createElement('script');
  utility.src = 'style/utility.js';
  
  utility.onload = () => {
    // Infrastructure ready; begin module sequence
    load_next(0);
  };

  utility.onerror = () => {
    console.error("StyleRT: Critical failure - utility.js missing.");
  };

  document.head.appendChild(utility);

  // 2. The Chain Loader
  const load_next = (index) => {
    if (index >= modules.length) {
      run_style();
      return;
    }
    
    const src = modules[index];

    // Accessing the property live so it doesn't matter if it was set late
    if (RT.debug) RT.debug.log('style', `Loading: ${src}`);

    const script = document.createElement('script');
    script.src = src;
    script.onload = () => load_next(index + 1);
    script.onerror = () => { 
      console.error(`StyleRT: Failed load on ${src}`); 
      load_next(index + 1); 
    };
    document.head.appendChild(script);
  };

  // 3. Phase 1: Semantics
  const run_style = () => {
    RT.debug.log('style', 'Starting Phase 1: Setup & Semantics');

    if(RT.article_generic) RT.article_generic();
    if(RT.RT_term) RT.RT_term();
    if(RT.RT_math) RT.RT_math();
    if(RT.RT_code) RT.RT_code();

    // Hand off to MathJax task queue
    if (window.MathJax && MathJax.Hub && MathJax.Hub.Queue) {
      RT.debug.log('style', 'MathJax detected. Queueing layout tasks...');
      MathJax.Hub.Queue(["Typeset", MathJax.Hub], continue_style);
    } else {
      continue_style();
    }
  };

  // 4. Phase 2: Layout
  const continue_style = () => {
    RT.debug.log('style', 'Starting Phase 2: Layout & Reveal');
    
    if(RT.RT_TOC) RT.RT_TOC();
    if(RT.paginate_by_element) RT.paginate_by_element();
    if(RT.page) RT.page();
    if(RT.body_visibility_visible) RT.body_visibility_visible();
    
    RT.debug.log('style', 'Style execution complete.');
  };
};
