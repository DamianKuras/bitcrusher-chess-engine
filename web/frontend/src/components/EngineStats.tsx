import { Chess } from 'chess.js'
import type { Square } from 'react-chessboard/dist/chessboard/types'

interface EngineStatsProps {
  depth: number | null
  nodes: number | null
  nps: number | null
  elapsed_ms: number | null
  pv: string[]
  startFen?: string
  showPv?: boolean
}

function fmt(n: number): string {
  if (n >= 1_000_000) return `${(n / 1_000_000).toFixed(1)}M`
  if (n >= 1_000) return `${Math.round(n / 1_000)}K`
  return String(n)
}

function pvToSan(startFen: string, pvMoves: string[]): string {
  try {
    const c = new Chess(startFen)
    const san: string[] = []
    for (const uci of pvMoves) {
      const move = c.move({
        from: uci.slice(0, 2) as Square,
        to: uci.slice(2, 4) as Square,
        promotion: uci[4] as 'q' | 'r' | 'b' | 'n' | undefined,
      })
      if (!move) break
      san.push(move.san)
    }
    return san.join(' ')
  } catch {
    return pvMoves.join(' ')
  }
}

export default function EngineStats({ depth, nodes, nps, elapsed_ms, pv, startFen, showPv = true }: EngineStatsProps) {
  const hasStats = nodes !== null

  return (
    <div className="bg-surface border border-line rounded-xl overflow-hidden">
      <div className="px-4 py-2.5 border-b border-line text-[11px] font-semibold uppercase tracking-widest text-muted">
        Engine Info
      </div>

      {!hasStats ? (
        <p className="px-4 py-3 text-[13px] text-muted">No search yet</p>
      ) : (
        <div className="p-4 flex flex-col gap-3">
          <div className="grid grid-cols-2 gap-x-4 gap-y-2">
            {([
              ['Depth',      String(depth)],
              ['Evaluated',  fmt(nodes!)],
              ['Per second', fmt(nps!)],
              ['Time',       `${elapsed_ms}ms`],
            ] as [string, string][]).map(([label, value]) => (
              <div key={label} className="flex flex-col gap-0.5">
                <span className="text-[10px] font-semibold uppercase tracking-wider text-muted">{label}</span>
                <span className="text-[14px] font-semibold text-copy tabular-nums">{value}</span>
              </div>
            ))}
          </div>

          {showPv && pv.length > 0 && (
            <div className="flex flex-col gap-1 pt-1 border-t border-line">
              <span className="text-[10px] font-semibold uppercase tracking-wider text-muted">Best line</span>
              <p className="text-[12px] text-accent font-mono leading-relaxed break-all">
                {startFen ? pvToSan(startFen, pv) : pv.join(' ')}
              </p>
            </div>
          )}
        </div>
      )}
    </div>
  )
}
