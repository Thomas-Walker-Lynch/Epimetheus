/*
  General utilities for the StyleRT library.
  Includes:
  1. Token-based Debugging System
  2. Physics (Ink metrics, Color analysis)
  3. Text Analysis
*/

window.StyleRT = window.StyleRT || {};

// --- DEBUG SYSTEM ---
window.StyleRT.debug = {
  // Add tokens here to enable specific logs: 'RT_code', 'physics', 'pipeline', 'layout'
  active_tokens: new Set(['pipeline', 'layout']),

  log: function(token, message) {
    if (this.active_tokens.has(token)) {
      console.log(`[StyleRT:${token}]`, message);
    }
  },

  warn: function(token, message) {
    if (this.active_tokens.has(token)) {
      console.warn(`[StyleRT:${token}]`, message);
    }
  },
  
  // Helper to enable/disable on the fly from console
  enable: function(token) { this.active_tokens.add(token); console.log(`Enabled: ${token}`); },
  disable: function(token) { this.active_tokens.delete(token); console.log(`Disabled: ${token}`); }
};

// --- UTILITIES ---
window.StyleRT.utility = {
  
  // --- FONT PHYSICS ---
  measure_ink_ratio: function(target_font, ref_font = null) {
    const debug = window.StyleRT.debug;
    debug.log('physics', `Measuring ink ratio for ${target_font}`);

    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');

    if (!ref_font) {
      const bodyStyle = window.getComputedStyle(document.body);
      ref_font = bodyStyle.fontFamily;
    }

    const get_metrics = (font) => {
      ctx.font = '100px ' + font; 
      const metrics = ctx.measureText('M');
      return {
        ascent: metrics.actualBoundingBoxAscent, 
        descent: metrics.actualBoundingBoxDescent 
      };
    };

    const ref_m = get_metrics(ref_font);
    const target_m = get_metrics(target_font);
    
    const ratio = ref_m.ascent / target_m.ascent;
    debug.log('physics', `Ink Ratio calculated: ${ratio.toFixed(3)}`);

    return { 
      ratio: ratio,
      baseline_diff: ref_m.descent - target_m.descent 
    };
  },

  // --- COLOR PHYSICS ---
  is_color_light: function(color_string) {
    const debug = window.StyleRT.debug;
    
    // 1. HSL Check
    if (color_string.startsWith('hsl')) {
      const numbers = color_string.match(/\d+/g);
      if (numbers && numbers.length >= 3) {
        const lightness = parseInt(numbers[2]);
        const is_light = lightness > 50;
        debug.log('color_physics', `HSL ${color_string} -> Lightness ${lightness}% -> ${is_light ? 'LIGHT' : 'DARK'}`);
        return is_light;
      }
    }

    // 2. RGB Check
    const rgb = color_string.match(/\d+/g);
    if (!rgb) {
      debug.warn('color_physics', `Failed to parse color: "${color_string}". Defaulting to Light.`);
      return true; 
    }

    const r = parseInt(rgb[0]);
    const g = parseInt(rgb[1]);
    const b = parseInt(rgb[2]);
    const luma = (r * 299 + g * 587 + b * 114) / 1000;
    const is_light = luma > 128;
    
    debug.log('color_physics', `RGB (${r},${g},${b}) -> Luma ${luma.toFixed(1)} -> ${is_light ? 'LIGHT' : 'DARK'}`);
    return is_light;
  },

  // --- TEXT ANALYSIS ---
  is_block_content: function(element) {
    return element.textContent.trim().includes('\n');
  }
};
