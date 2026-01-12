/*
  Layout Paginator: paginate_by_element
  Measures heights and groups elements.
  Logic: Headings must stay with the element immediately following them.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.paginate_by_element = function() {
  const RT = window.StyleRT;
  const body = document.body;
  const limit = RT.layout.page_height || 1000; 

  const elements = Array.from(body.children).filter(el => 
    el.tagName !== 'SCRIPT' && el.tagName !== 'STYLE' && el.tagName !== 'RT-PAGE'
  );

  const pages = [];
  let current_batch = [];
  let current_h = 0;

  const get_el_height = (el) => {
    const rect = el.getBoundingClientRect();
    const style = window.getComputedStyle(el);
    const margin = parseFloat(style.marginTop) + parseFloat(style.marginBottom);
    return rect.height + margin;
  };

  for (let i = 0; i < elements.length; i++) {
    const el = elements[i];
    const h = get_el_height(el);
    const is_heading = /H[1-6]/.test(el.tagName);

    // Lookahead: If this is a heading, check the next element too
    let total_required_h = h;
    if (is_heading && i + 1 < elements.length) {
      total_required_h += get_el_height(elements[i + 1]);
    }

    // If the element (or heading + next) exceeds limit, start new page
    if (current_h + total_required_h > limit && current_batch.length > 0) {
      pages.push(current_batch);
      current_batch = [];
      current_h = 0;
    }

    current_batch.push(el);
    current_h += h;
  }

  if (current_batch.length > 0) pages.push(current_batch);

  // Rebuild DOM
  const assets = Array.from(document.querySelectorAll('script, style'));
  body.innerHTML = '';
  pages.forEach(list => {
    const page_el = document.createElement('rt-page');
    list.forEach(item => page_el.appendChild(item));
    body.appendChild(page_el);
  });
  assets.forEach(a => body.appendChild(a));

  if (RT.debug) RT.debug.log('layout', `Pagination complete: ${pages.length} pages.`);
};
