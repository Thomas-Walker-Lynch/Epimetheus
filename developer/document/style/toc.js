/*
  Generates a Table of Contents.
  Inserts it at the very top of the document body.
*/
window.StyleRT = window.StyleRT || {};

window.StyleRT.toc = function() {
  const RT = window.StyleRT;
  // Use 'pipeline' token if available, otherwise silent
  const debug = RT.debug ? RT.debug.log.bind(RT.debug) : () => {};

  debug('pipeline', 'Generating TOC...');

  // 1. Create Container (reusing 'page' class so it picks up your gold style)
  const container = document.createElement('div');
  container.className = 'page';
  container.id = 'rt-toc';

  // 2. Title
  const title = document.createElement('h1');
  title.textContent = 'Table of Contents';
  title.style.textAlign = 'center';
  container.appendChild(title);

  // 3. Build List
  const list = document.createElement('ul');
  document.querySelectorAll('h1').forEach((header, i) => {
    if (header === title) return;
    if (!header.id) header.id = `section-${i}`;

    const li = document.createElement('li');
    const a = document.createElement('a');
    a.href = `#${header.id}`;
    a.textContent = header.textContent;
    a.style.textDecoration = 'none';
    a.style.color = 'inherit';
    
    li.appendChild(a);
    list.appendChild(li);
  });
  
  container.appendChild(list);

  // 4. Insert at Top (Prepend)
  document.body.insertBefore(container, document.body.firstChild);
};
