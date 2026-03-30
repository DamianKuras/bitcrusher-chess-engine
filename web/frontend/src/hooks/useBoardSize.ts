import { useEffect, useState } from 'react'

/** Returns a responsive board size (px), capped at `max`. */
export function useBoardSize(max = 520): number {
  function calc(): number {
    // 48px page padding + 38px eval bar + gap = ~86px reserved beside the board
    return Math.min(max, Math.max(280, window.innerWidth - 86))
  }
  const [size, setSize] = useState(calc)
  useEffect(() => {
    const handler = () => setSize(calc())
    window.addEventListener('resize', handler)
    return () => window.removeEventListener('resize', handler)
  }, [])
  return size
}
