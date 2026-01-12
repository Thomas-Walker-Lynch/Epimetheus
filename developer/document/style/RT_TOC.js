/*
  Processes existing <RT-TOC> tags.
  Populates each with headings found below it that are exactly one level deeper.
  Stops scanning if a heading of the same or higher level is encountered.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.RT_TOC = function() {
  const debug = window.StyleRT.debug;
  const toc_tags = document.querySelectorAll('rt-toc');

  toc_tags.forEach((container, toc_index) => {
    container.style.display = 'block';
    
    // 1. Determine the context level of this TOC
    // We look backward to find the most recent heading (H1-H6)
    let context_level = 0; // Default to 0 (looking for H1s)
    let prev = container.previousElementSibling;
    
    while (prev) {
      const match = prev.tagName.match(/^H([1-6])$/);
      if (match) {
        context_level = parseInt(match[1]);
        break;
      }
      prev = prev.previousElementSibling;
    }

    const target_level = context_level + 1;
    const stop_at_or_above = context_level; 
    
    if (debug) debug.log('RT_TOC', `TOC #${toc_index} context: H${context_level}. Targeting: H${target_level}`);

    // 2. Setup Internal Structure
    container.innerHTML = ''; // Clear any placeholder content
    const title = document.createElement('h1');
    title.textContent = context_level === 0 ? 'Table of Contents' : 'Section Contents';
    title.style.textAlign = 'center';
    container.appendChild(title);

    const list = document.createElement('ul');
    list.style.listStyle = 'none';
    list.style.paddingLeft = '0';
    container.appendChild(list);

    // 3. Scan Forward for headings
    let next_el = container.nextElementSibling;
    while (next_el) {
      const match = next_el.tagName.match(/^H([1-6])$/);
      if (match) {
        const found_level = parseInt(match[1]);

        // STOP condition: We hit a heading at our level or higher
        // (e.g., if we are under an H1 looking for H2s, we stop at the next H1)
        if (context_level !== 0 && found_level <= stop_at_or_above) {
          break;
        }

        // COLLECT condition: Heading is exactly one level deeper
        if (found_level === target_level) {
          if (!next_el.id) next_el.id = `toc-ref-${toc_index}-${found_level}-${list.children.length}`;

          const li = document.createElement('li');
          li.style.marginBottom = '0.5rem';

          const a = document.createElement('a');
          a.href = `#${next_el.id}`;
          a.textContent = next_el.textContent;
          a.style.textDecoration = 'none';
          a.style.color = 'inherit';
          a.style.display = 'block';

          // Hover effects
          a.onmouseover = () => a.style.color = 'hsl(42, 100%, 50%)';
          a.onmouseout = () => a.style.color = 'inherit';

          li.appendChild(a);
          list.appendChild(li);
        }
      }
      next_el = next_el.nextElementSibling;
    }
  });
};
