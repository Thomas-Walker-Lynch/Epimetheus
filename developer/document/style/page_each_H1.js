/*
  Wraps child elements into pages based on H1 headers.
*/
function page_each_H1(){
  const body = document.body;
  const elements = Array.from(body.children);
  const pages = [];
  let current_page_elements = [];

  elements.forEach(el => {
    if ( el.tagName === 'SCRIPT' || el.tagName === 'STYLE' ){
      return;
    }

    if (el.tagName === 'H1'){
      if (current_page_elements.length > 0){
        pages.push(current_page_elements);
      }
      current_page_elements = [el];
    } else {
      current_page_elements.push(el);
    }
  });

  if (current_page_elements.length > 0){
    pages.push(current_page_elements);
  }

  const scripts = Array.from(document.querySelectorAll('script'));
  body.innerHTML = '';

  pages.forEach(element_list => {
    const page_div = document.createElement('div');
    page_div.className = 'page';
    element_list.forEach(el => page_div.appendChild(el));
    body.appendChild(page_div);
  });

  scripts.forEach(s => body.appendChild(s));
}

window.StyleRT = window.StyleRT || {};
window.StyleRT.page_each_H1 = page_each_H1;
