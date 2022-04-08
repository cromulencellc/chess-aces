document.addEventListener('DOMContentLoaded', (_evt) => {
  for (const toggleform of document.getElementsByClassName('toggleform')) {
    const toggler = toggleform.getElementsByTagName('a')[0]
    const form = toggleform.getElementsByTagName('form')[0]

    const orig_form_display = form.style['display']

    toggler.addEventListener('click', (evt) => {
      toggler.style['display'] = 'none'
      form.style['display'] = orig_form_display

      evt.preventDefault()
    })

    toggler.style['display'] = 'inline'
    form.style['display'] = 'none'
  }
})
