export interface SearchResult {
  fen: string
  depth: number
  score_cp?: number | null
  score_mate?: number | null
  best_move: string
  pv: string[]
  nodes: number
  elapsed_ms: number
  nps: number
}

export async function searchPosition(fen: string, depth: number): Promise<SearchResult> {
  const res = await fetch('/search', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ fen, depth }),
  })
  if (!res.ok) throw new Error(`Search failed: ${res.status}`)
  return res.json()
}
