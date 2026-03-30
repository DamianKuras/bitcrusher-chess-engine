interface MoveHistoryProps {
  moves: string[]
}

export default function MoveHistory({ moves }: MoveHistoryProps) {
  const pairs: [number, string, string?][] = []
  for (let i = 0; i < moves.length; i += 2) {
    pairs.push([Math.floor(i / 2) + 1, moves[i], moves[i + 1]])
  }

  const lastPair  = pairs.length - 1
  const whiteIsLast = moves.length % 2 === 1

  return (
    <div className="bg-surface border border-line rounded-xl overflow-hidden flex flex-col">
      <div className="px-4 py-2.5 border-b border-line text-[11px] font-semibold uppercase tracking-widest text-muted shrink-0">
        Move History
      </div>
      <div className="h-60 overflow-y-auto px-2 py-2">
        {pairs.length === 0 ? (
          <p className="text-[13px] text-muted px-2 py-1">Game not started</p>
        ) : (
          pairs.map(([num, white, black]) => (
            <div key={num} className="grid grid-cols-[26px_1fr_1fr] rounded px-1 py-0.5 hover:bg-line/40 transition-colors">
              <span className="text-[12px] text-muted pt-0.5">{num}.</span>
              <span className={[
                'text-[13px] font-medium px-1 rounded',
                num === lastPair + 1 && whiteIsLast ? 'bg-accent text-white' : 'text-copy',
              ].join(' ')}>
                {white}
              </span>
              <span className={[
                'text-[13px] font-medium px-1 rounded',
                black && num === lastPair + 1 && !whiteIsLast ? 'bg-accent text-white' : 'text-copy',
              ].join(' ')}>
                {black ?? ''}
              </span>
            </div>
          ))
        )}
      </div>
    </div>
  )
}
