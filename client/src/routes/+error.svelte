<script lang='ts'>
  import { page } from '$app/stores'
</script>

<div class='flex place-content-center place-items-center' style='height: calc(100vh - 8rem)'>
  <h1 class='text-6xl'>{$page.status} — {$page.error?.message}</h1>
</div>
