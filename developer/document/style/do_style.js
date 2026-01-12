/*
  Master Loader & Orchestrator for StyleRT.
  
  PIPELINE DEPENDENCY CHAIN:
  1. PHYSICS (Utility, Theme) -> Required by everything.
  2. GLOBAL (Article)         -> Sets background for contrast checks.
  3. SEMANTICS (Code, Math)   -> Expands text size; must run before pagination.
  4. STRUCTURE (H1, TOC)      -> Creates .page containers.
  5. STYLING (Page)           -> Applies borders/margins to .page containers.
  6. REVEAL                   -> Shows the result.
*/

window.StyleRT = window.StyleRT || {};

window.StyleRT.do_style = function() {
  
  // 1. Manifest
  const modules = [
    'style/utility.js',         
    'style/theme_RT.js',        
    'style/RT_term.js',         
    'style/RT_math.js',         
    'style/RT_code.js',         
    'style/article_generic.js', 
    'style/page_each_H1.js',    
    'style/toc.js',             
    'style/page.js',            
    'style/body_visibility_visible.js' 
  ];

  // 2. Loader
  const load_next = (index) => {
    if (index >= modules.length) {
      run_pipeline();
      return;
    }
    const src = modules[index];
    const script = document.createElement('script');
    script.src = src;
    script.onload = () => load_next(index + 1);
    script.onerror = () => { console.error(`StyleRT: Failed ${src}`); load_next(index + 1); };
    document.head.appendChild(script);
  };

  // 3. Execution Pipeline
  const run_pipeline = () => {
    const RT = window.StyleRT;
    const debug = RT.debug;

    debug.log('pipeline', 'Starting execution...');

    // PHASE 1: GLOBAL SETUP
    if(RT.article_generic) {
      debug.log('pipeline', 'Phase 1: Global Setup (article_generic)');
      RT.article_generic();
    }

    // PHASE 2: SEMANTICS (In-place modification)
    debug.log('pipeline', 'Phase 2: Semantics (Term, Math, Code)');
    if(RT.RT_term) RT.RT_term();
    if(RT.RT_math) RT.RT_math();
    if(RT.RT_code) RT.RT_code();

    // PHASE 3: STRUCTURE (DOM Surgery)
    debug.log('pipeline', 'Phase 3: Structure (Pagination & TOC)');
    
    // A. Content Pagination (Breaks body into pages)
    if(RT.page_each_H1) RT.page_each_H1();
    
    // B. TOC Generation (Creates the TOC Page)
    // Must run AFTER H1s are processed (or scan them), but definitely creates a .page
    if(RT.toc) RT.toc();

    // PHASE 4: STYLING (The Frame)
    // Must run LAST, after all .page elements (Content + TOC) exist.
    if(RT.page) {
      debug.log('pipeline', 'Phase 4: Styling (Applying Page Borders)');
      RT.page();
    }

    // PHASE 5: REVEAL
    debug.log('pipeline', 'Phase 5: Reveal');
    if(RT.body_visibility_visible) RT.body_visibility_visible();
    
    debug.log('pipeline', 'Execution complete.');
  };

  load_next(0);
};
