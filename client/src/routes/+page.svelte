<script lang='ts'>
  import { invoke } from '@tauri-apps/api/tauri'

  const PORT = 52617
  let address: string
  let port: string
  let err = ''
  let color = 'slate'
</script>

<label for='address'>IP address</label>
<input type='text' name='address' required maxlength=39 bind:value={address}>
<label for='port'>Port</label>
<input type='text' inputmode='numeric' name='port' maxlength=5
  pattern='[^\D0]\d{'{'}0,4{'}'}' placeholder={PORT.toString()}
  bind:value={port}>

<button on:click={async () => {
  err = 'Connecting...'
  color = 'slate'

  try {
    await invoke('connect', { address, port: parseInt(port) || PORT })
    err = 'Connected!'
    color = 'green'
  }
  catch (e) {
    // @ts-ignore
    err = e
    color = 'red'
  }
}} class='text-xl'>Connect</button>
<span class='text-sm text-{color}-500'>{err}</span>
