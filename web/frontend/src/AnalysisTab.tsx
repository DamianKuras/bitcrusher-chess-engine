import React, { useRef, useState } from 'react'
import { Chess } from 'chess.js'
import { Chessboard } from 'react-chessboard'
import type { Square } from 'react-chessboard/dist/chessboard/types'

import { searchPosition, type SearchResult } from './api'
import { useBoardSize } from './hooks/useBoardSize'
import EvalBar from './components/EvalBar'
import EngineStats from './components/EngineStats'

const STARTING_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

interface LastMove { from: string; to: string }

function getKingSquare(chess: Chess): string | null {
  const turn = chess.turn()
  for (const row of chess.board())
    for (const piece of row)
      if (piece && piece.type === 'k' && piece.color === turn) return piece.square
  return null
}

function buildSquareStyles(lm: LastMove | null, inCheck: boolean, kingSq: string | null): Record<string, React.CSSProperties> {
  const s: Record<string, React.CSSProperties> = {}
  if (lm) {
    s[lm.from] = { backgroundColor: 'rgba(255,210,0,0.28)' }
    s[lm.to]   = { backgroundColor: 'rgba(255,210,0,0.52)' }
  }
  if (inCheck && kingSq) s[kingSq] = { backgroundColor: 'rgba(220,50,50,0.65)' }
  return s
}

const btnSecondary = 'py-1.5 px-3 rounded-lg bg-line/60 hover:bg-line text-copy text-sm font-medium transition-colors'

export default function AnalysisTab() {
  const chess     = useRef(new Chess())
  const boardSize = useBoardSize()

  const [boardFen,        setBoardFen]        = useState(STARTING_FEN)
  const [fenInput,        setFenInput]        = useState(STARTING_FEN)
  const [fenError,        setFenError]        = useState<string | null>(null)
  const [boardOrientation,setBoardOrientation]= useState<'white' | 'black'>('white')
  const [depth,           setDepth]           = useState(10)
  const [isAnalyzing,     setIsAnalyzing]     = useState(false)
  const [result,          setResult]          = useState<SearchResult | null>(null)
  const [lastMove,        setLastMove]        = useState<LastMove | null>(null)
  const [evalCp,          setEvalCp]          = useState<number | null>(null)
  const [evalMate,        setEvalMate]        = useState<number | null>(null)

  function loadFenFromInput(fen: string): boolean {
    try { chess.current.load(fen.trim()) } catch { setFenError('Invalid FEN'); return false }
    setFenError(null)
    const canonical = chess.current.fen()
    setBoardFen(canonical); setFenInput(canonical); setResult(null)
    return true
  }

  async function handleAnalyze() {
    let fenToSearch = boardFen
    if (fenInput.trim() !== boardFen) {
      if (!loadFenFromInput(fenInput)) return
      fenToSearch = chess.current.fen()
    }
    setResult(null); setIsAnalyzing(true)
    try {
      const r = await searchPosition(fenToSearch, depth)
      setResult(r)
      const stm = fenToSearch.split(' ')[1]
      setEvalCp(stm === 'w' ? (r.score_cp ?? null) : r.score_cp != null ? -r.score_cp : null)
      setEvalMate(stm === 'w' ? (r.score_mate ?? null) : r.score_mate != null ? -r.score_mate : null)
    } catch {
      setFenError('Search failed - check the API is running')
    } finally {
      setIsAnalyzing(false)
    }
  }

  function onDrop(source: Square, target: Square): boolean {
    try { chess.current.move({ from: source, to: target, promotion: 'q' }) } catch { return false }
    const newFen = chess.current.fen()
    setBoardFen(newFen); setFenInput(newFen)
    setLastMove({ from: source, to: target })
    setResult(null); setFenError(null)
    return true
  }

  function resetToStart() {
    chess.current.reset()
    const fen = chess.current.fen()
    setBoardFen(fen); setFenInput(fen)
    setFenError(null); setLastMove(null); setResult(null)
    setEvalCp(null); setEvalMate(null)
  }

  const bestMoveArrow: [Square, Square][] = result?.best_move
    ? [[result.best_move.slice(0, 2) as Square, result.best_move.slice(2, 4) as Square]]
    : []

  const inCheck      = chess.current.inCheck()
  const squareStyles = buildSquareStyles(lastMove, inCheck, inCheck ? getKingSquare(chess.current) : null)

  return (
    <main className="flex gap-6 p-6 justify-center items-start flex-1 flex-wrap">

      {/* ── Board ────────────────────────────────────────── */}
      <div className="flex gap-3 items-start">
        <EvalBar scoreCp={evalCp} scoreMate={evalMate} isWhitePerspective size={boardSize} />
        <div className="select-none">
          <Chessboard
            id="analysis-board"
            position={boardFen}
            onPieceDrop={onDrop}
            boardOrientation={boardOrientation}
            boardWidth={boardSize}
            customDarkSquareStyle={{ backgroundColor: '#b58863' }}
            customLightSquareStyle={{ backgroundColor: '#f0d9b5' }}
            customSquareStyles={squareStyles}
            customArrows={bestMoveArrow}
            customArrowColor="rgba(94,156,240,0.8)"
            arePiecesDraggable={!isAnalyzing}
          />
        </div>
      </div>

      {/* ── Sidebar ──────────────────────────────────────── */}
      <aside className="flex flex-col gap-3 w-72">

        {/* FEN + controls card */}
        <div className="bg-surface border border-line rounded-xl p-4 flex flex-col gap-4">
          <div className="flex flex-col gap-2">
            <label className="text-[11px] font-semibold uppercase tracking-widest text-muted">Position (FEN)</label>
            <textarea
              className="w-full bg-bg border border-line rounded-lg text-[11px] font-mono text-copy p-2.5 resize-none focus:outline-none focus:border-accent leading-relaxed transition-colors"
              rows={3}
              value={fenInput}
              onChange={e => setFenInput(e.target.value)}
              onKeyDown={e => { if (e.key === 'Enter') { e.preventDefault(); handleAnalyze() } }}
              spellCheck={false}
            />
            {fenError && <p className="text-[11px] text-danger">{fenError}</p>}
          </div>

          <div className="flex flex-col gap-2">
            <label className="text-[11px] font-semibold uppercase tracking-widest text-muted">
              Depth <span className="text-copy font-bold">{depth}</span>
            </label>
            <input type="range" min={1} max={20} value={depth} onChange={e => setDepth(Number(e.target.value))} />
          </div>

          <div className="flex gap-2 flex-wrap">
            <button
              onClick={handleAnalyze}
              disabled={isAnalyzing}
              className="flex-1 py-1.5 rounded-lg bg-accent hover:bg-accent-h disabled:opacity-50 text-white text-sm font-semibold transition-colors"
            >
              {isAnalyzing ? 'Analyzing…' : 'Analyze'}
            </button>
            <button onClick={resetToStart} className={btnSecondary}>Reset</button>
            <button onClick={() => setBoardOrientation(o => o === 'white' ? 'black' : 'white')} className={btnSecondary}>
              Flip
            </button>
          </div>
        </div>

        <EngineStats
          depth={result ? depth : null}
          nodes={result?.nodes ?? null}
          nps={result?.nps ?? null}
          elapsed_ms={result?.elapsed_ms ?? null}
          pv={result?.pv ?? []}
          startFen={result ? boardFen : undefined}
        />
      </aside>
    </main>
  )
}
