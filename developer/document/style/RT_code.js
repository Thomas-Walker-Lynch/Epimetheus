/*
  Processes <RT-CODE> tags using StyleRT utilities.
  Instrumented with token-based debugging.
*/
function RT_code() {
  const RT = window.StyleRT;
  const U = RT.utility; 
  const debug = RT.debug;

  debug.log('RT_code', 'Starting render cycle.');

  // 1. Fetch Theme
  const theme = RT.active_theme ? RT.active_theme() : {};
  const accent = theme.accent || 'gold'; 

  // 2. Physics
  const metrics = U.measure_ink_ratio('monospace');
  
  document.querySelectorAll('rt-code').forEach((el, index) => {
    el.style.fontFamily = 'monospace';
    
    // Analysis
    const is_block = U.is_block_content(el);
    const parentStyle = window.getComputedStyle(el.parentElement);
    const parentColor = parentStyle.color;
    
    // Physics
    const is_text_light = U.is_color_light(parentColor);
    
    if (index === 0) {
      // Always log the first one if token is active, for sanity check
      debug.log('RT_code', `Sample #0: Parent Color "${parentColor}" -> Detect Light? ${is_text_light}`);
    }

    // Visuals
    const alpha = is_block ? 0.08 : 0.15;
    const overlay = is_text_light ? `rgba(255,255,255,${alpha})` : `rgba(0,0,0,${alpha})`;
    const text_color = is_text_light ? '#ffffff' : '#000000';

    el.style.backgroundColor = overlay;

    if (is_block) {
      el.style.display = 'block';
      el.style.whiteSpace = 'pre';
      el.style.fontSize = (parseFloat(parentStyle.fontSize) * metrics.ratio * 0.95) + 'px'; 
      el.style.padding = '1.2rem';
      el.style.margin = '1.5rem 0';
      el.style.borderLeft = `4px solid ${accent}`;
      el.style.color = 'inherit'; 
    } else {
      el.style.display = 'inline';
      const exactPx = parseFloat(parentStyle.fontSize) * metrics.ratio * 1.0; 
      el.style.fontSize = exactPx + 'px';
      el.style.padding = '0.1rem 0.35rem';
      el.style.borderRadius = '3px';
      const offsetPx = metrics.baseline_diff * (exactPx / 100);
      el.style.verticalAlign = offsetPx + 'px';
      el.style.color = text_color; 
    }
  });
  
  debug.log('RT_code', 'Render cycle complete.');
}

window.StyleRT = window.StyleRT || {};
window.StyleRT.RT_code = RT_code;
