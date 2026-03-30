import { useState } from 'react'

interface GameControlsProps {
  depth: number
  onDepthChange: (d: number) => void
  playerColor: 'white' | 'black'
  onPlayerColorChange: (c: 'white' | 'black') => void
  onNewGame: () => void
  onFlipBoard: () => void
  onGetFen: () => string
  onGetPgn: () => string
  disabled: boolean
}

export default function GameControls({
  depth, onDepthChange, playerColor, onPlayerColorChange,
  onNewGame, onFlipBoard, onGetFen, onGetPgn, disabled,
}: GameControlsProps) {
  const [copiedFen, setCopiedFen] = useState(false)
  const [copiedPgn, setCopiedPgn] = useState(false)

  function copy(text: string, set: (v: boolean) => void) {
    navigator.clipboard.writeText(text).then(() => { set(true); setTimeout(() => set(false), 1500) })
  }

  const btnBase = 'py-1.5 rounded-lg text-sm font-medium transition-all'
  const btnSecondary = `${btnBase} bg-line/60 hover:bg-line text-copy`

  return (
    <div className="bg-surface border border-line rounded-xl p-4 flex flex-col gap-4">

      {/* Play as */}
      <div className="flex flex-col gap-2">
        <label className="text-[11px] font-semibold uppercase tracking-widest text-muted">Play as</label>
        <div className="flex gap-2">
          {(['white', 'black'] as const).map(c => (
            <button
              key={c}
              onClick={() => onPlayerColorChange(c)}
              disabled={disabled}
              className={[
                'flex-1 py-1.5 rounded-lg border text-sm font-medium transition-all',
                'disabled:opacity-40 disabled:cursor-not-allowed',
                playerColor === c
                  ? 'bg-accent border-accent text-white'
                  : 'border-line text-muted hover:border-accent hover:text-copy',
              ].join(' ')}
            >
              {c === 'white' ? '♔ White' : '♚ Black'}
            </button>
          ))}
        </div>
      </div>

      {/* Depth */}
      <div className="flex flex-col gap-2">
        <label className="text-[11px] font-semibold uppercase tracking-widest text-muted">
          Depth <span className="text-copy font-bold">{depth}</span>
        </label>
        <input type="range" min={1} max={20} value={depth} onChange={e => onDepthChange(Number(e.target.value))} />
      </div>

      {/* Actions */}
      <div className="grid grid-cols-2 gap-2">
        <button
          onClick={onNewGame}
          className={`col-span-2 ${btnBase} bg-accent hover:bg-accent-h text-white font-semibold`}
        >
          New Game
        </button>
        <button onClick={onFlipBoard} className={btnSecondary}>Flip Board</button>
        <button onClick={() => copy(onGetFen(), setCopiedFen)} className={btnSecondary}>
          {copiedFen ? '✓ FEN' : 'Copy FEN'}
        </button>
        <button onClick={() => copy(onGetPgn(), setCopiedPgn)} className={`col-span-2 ${btnSecondary}`}>
          {copiedPgn ? '✓ PGN copied' : 'Copy PGN'}
        </button>
      </div>

    </div>
  )
}
