document.addEventListener('DOMContentLoaded', (_evt) => {
  for (const confirm_form of document.getElementsByClassName('confirmform')) {
    const mesg = confirm_form.dataset.confirm || 'Are you sure?'
    confirm_form.addEventListener('submit', (submission_event) => {
      let did_confirm = confirm(mesg)
      if (did_confirm) return
      submission_event.preventDefault()
    })
  }
})
