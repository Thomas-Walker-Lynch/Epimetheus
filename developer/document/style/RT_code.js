/*
  Processes <RT-CODE> tags.
  Uses the central config or CSS variables from the theme.
*/
function RT_code() {
  const RT = window.StyleRT;
  const U = RT.utility; 
  const debug = RT.debug;

  debug.log('RT_code', 'Starting render cycle.');

  const metrics = U.measure_ink_ratio('monospace');
  
  // Scoped Selector: find code blocks anywhere, but styling relies on inheritance
  document.querySelectorAll('rt-code').forEach((el) => {
    el.style.fontFamily = 'monospace';
    
    // Check context for accent color (CSS Variable fallback)
    const computed = window.getComputedStyle(el);
    const accent = computed.getPropertyValue('--rt-accent').trim() || 'gold';
    
    const is_block = U.is_block_content(el);
    const parentColor = computed.color;
    const is_text_light = U.is_color_light(parentColor);
    
    const alpha = is_block ? 0.08 : 0.15;
    const overlay = is_text_light ? `rgba(255,255,255,${alpha})` : `rgba(0,0,0,${alpha})`;
    const text_color = is_text_light ? '#ffffff' : '#000000';

    el.style.backgroundColor = overlay;

    if (is_block) {
      el.style.display = 'block';
      el.style.whiteSpace = 'pre';
      el.style.fontSize = (parseFloat(computed.fontSize) * metrics.ratio * 0.95) + 'px'; 
      el.style.padding = '1.2rem';
      el.style.margin = '1.5rem 0';
      el.style.borderLeft = `4px solid ${accent}`;
      el.style.color = 'inherit'; 
    } else {
      el.style.display = 'inline';
      const exactPx = parseFloat(computed.fontSize) * metrics.ratio * 1.0; 
      el.style.fontSize = exactPx + 'px';
      el.style.padding = '0.1rem 0.35rem';
      el.style.borderRadius = '3px';
      // Vertical align fix for inline code
      const offsetPx = metrics.baseline_diff * (exactPx / 100);
      el.style.verticalAlign = offsetPx + 'px';
      el.style.color = text_color; 
    }
  });
  
  debug.log('RT_code', 'Render cycle complete.');
}

window.StyleRT = window.StyleRT || {};
window.StyleRT.RT_code = RT_code;
