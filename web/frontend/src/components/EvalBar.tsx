interface EvalBarProps {
  scoreCp: number | null
  scoreMate: number | null
  isWhitePerspective: boolean
  size?: number
}

function toWhitePct(cp: number): number {
  const clamped = Math.max(-1000, Math.min(1000, cp))
  return 50 + (clamped / 1000) * 45
}

function evalLabel(cp: number | null, mate: number | null): string {
  if (mate !== null) {
    if (mate === 0) return '½'
    return mate > 0 ? `+M${mate}` : `-M${Math.abs(mate)}`
  }
  if (cp === null) return '0.0'
  const sign = cp > 0 ? '+' : ''
  return sign + (cp / 100).toFixed(1)
}

export default function EvalBar({ scoreCp, scoreMate, isWhitePerspective, size = 520 }: EvalBarProps) {
  const whiteCp   = isWhitePerspective ? (scoreCp ?? 0) : -(scoreCp ?? 0)
  const whiteMate = isWhitePerspective ? scoreMate : scoreMate !== null ? -scoreMate : null

  let whitePct: number
  if (whiteMate !== null) {
    whitePct = whiteMate > 0 ? 95 : 5
  } else {
    whitePct = toWhitePct(whiteCp)
  }

  const label = evalLabel(whiteCp, whiteMate)

  return (
    <div className="flex flex-col items-center gap-2 shrink-0">
      <div
        className="w-[18px] rounded overflow-hidden flex flex-col border border-line"
        style={{ height: size }}
      >
        <div className="eval-black" style={{ height: `${100 - whitePct}%` }} />
        <div className="eval-white" style={{ height: `${whitePct}%` }} />
      </div>
      <span className="text-[11px] font-semibold text-muted tabular-nums">{label}</span>
    </div>
  )
}
