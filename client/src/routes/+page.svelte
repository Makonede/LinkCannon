<script lang='ts'>
  import { invoke } from '@tauri-apps/api/tauri'

  const PORT = 52617
  let address: string
  let portStr: string
  let err = ''
  let color = 'slate'
</script>

<label for='address'>IP address</label>
<input type='text' name='address' required maxlength=39 bind:value={address}>
<label for='port'>Port</label>
<input type='text' inputmode='numeric' name='port' maxlength=5
  pattern='[^\D0]\d{'{'}0,3{'}'}|[1-6]\d{'{'}4{'}'}' placeholder={`${PORT}`}
  bind:value={portStr}>

<button on:click={async () => {
  // Validate port
  const port = portStr ? parseInt(portStr) : PORT
  if (!port) {
    err = 'Invalid port'
    color = 'red'
    return
  }

  err = 'Connecting...'
  color = 'slate'

  try {
    await invoke('connect', { address, port })
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
