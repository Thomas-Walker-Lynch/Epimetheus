/*
  Processes <RT-TERM> and <RT-NEOLOGISM> tags.
  - Styles only the first occurrence of a unique term/neologism.
  - The "-em" variants (e.g., <RT-term-em>) are always styled.
  - Automatically generates IDs for first occurrences for future indexing.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.RT_term = function() {
  const RT = window.StyleRT;
  const debug = RT.debug || { log: function(){} };
  
  // Track seen terms to ensure only the first one gets decorated
  const seen_terms = new Set();

  // Helper to apply professional "Definiendum" styling
  const apply_style = (el, is_neologism) => {
    el.style.fontStyle = 'italic';
    el.style.fontWeight = is_neologism ? '600' : '500';
    el.style.color = is_neologism ? 'var(--rt-brand-secondary)' : 'var(--rt-brand-primary)';
    el.style.paddingRight = '0.1em'; // Compensation for italic slant
    el.style.display = 'inline';
  };

  // Selector covers all four variations
  const tags = document.querySelectorAll('rt-term, rt-term-em, rt-neologism, rt-neologism-em');

  tags.forEach(el => {
    const tag_name = el.tagName.toLowerCase();
    const is_neologism = tag_name.includes('neologism');
    const is_explicit_em = tag_name.endsWith('-em');
    
    // Normalize text for comparison (e.g., "Symbol" vs "symbol")
    const term_text = el.textContent.trim().toLowerCase();
    const slug = term_text.replace(/\s+/g, '-');

    if (is_explicit_em || !seen_terms.has(term_text)) {
      apply_style(el, is_neologism);
      
      // If it's the first occurrence, mark it and give it an ID
      if (!is_explicit_em && !seen_terms.has(term_text)) {
        seen_terms.add(term_text);
        if (!el.id) el.id = `def-${is_neologism ? 'neo-' : ''}${slug}`;
        debug.log('RT_term', `Defined first instance of: ${term_text}`);
      }
    } else {
      // For subsequent mentions that aren't "-em", we treat as normal prose
      el.style.fontStyle = 'normal';
      el.style.color = 'inherit';
      el.style.fontWeight = 'inherit';
    }
  });
};
