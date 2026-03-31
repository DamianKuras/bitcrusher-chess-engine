import React, { useCallback, useRef, useState } from 'react'
import { Chess } from 'chess.js'
import { Chessboard } from 'react-chessboard'
import type { Square } from 'react-chessboard/dist/chessboard/types'

import { searchPosition } from './api'
import { useBoardSize } from './hooks/useBoardSize'
import EvalBar from './components/EvalBar'
import MoveHistory from './components/MoveHistory'
import GameControls from './components/GameControls'
import EngineStats from './components/EngineStats'

interface LastMove { from: string; to: string }
interface Stats    { depth: number; nodes: number; nps: number; elapsed_ms: number; pv: string[]; startFen: string }

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

export default function PlayTab() {
  const chess     = useRef(new Chess())
  const boardSize = useBoardSize()

  const [fen,             setFen]             = useState(chess.current.fen())
  const [moves,           setMoves]           = useState<string[]>([])
  const [isThinking,      setIsThinking]      = useState(false)
  const [gameStatus,      setGameStatus]      = useState<string | null>(null)
  const [depth,           setDepth]           = useState(5)
  const [playerColor,     setPlayerColor]     = useState<'white' | 'black'>('white')
  const [boardOrientation,setBoardOrientation]= useState<'white' | 'black'>('white')
  const [stats,           setStats]           = useState<Stats | null>(null)
  const [lastMove,        setLastMove]        = useState<LastMove | null>(null)
  const [evalCp,          setEvalCp]          = useState<number | null>(null)
  const [evalMate,        setEvalMate]        = useState<number | null>(null)

  const isEngineTurn = useCallback(() => {
    const t = chess.current.turn()
    return playerColor === 'white' ? t === 'b' : t === 'w'
  }, [playerColor])

  const syncState = useCallback(() => {
    setFen(chess.current.fen())
    setMoves(chess.current.history())
  }, [])

  const gameOverMsg = useCallback((): string => {
    const c = chess.current
    if (c.isCheckmate())           return c.turn() === 'w' ? 'Black wins by checkmate' : 'White wins by checkmate'
    if (c.isStalemate())           return 'Draw - stalemate'
    if (c.isThreefoldRepetition()) return 'Draw - threefold repetition'
    if (c.isInsufficientMaterial())return 'Draw - insufficient material'
    if (c.isDraw())                return 'Draw'
    return ''
  }, [])

  const runEngineMove = useCallback(async (currentDepth: number) => {
    const engineTurn = chess.current.turn()
    const searchFen  = chess.current.fen()
    setIsThinking(true)
    try {
      const r = await searchPosition(searchFen, currentDepth)
      if (r.score_mate != null) {
        setEvalMate(engineTurn === 'b' ? -r.score_mate : r.score_mate)
        setEvalCp(null)
      } else {
        setEvalCp(engineTurn === 'b' ? -(r.score_cp ?? 0) : (r.score_cp ?? 0))
        setEvalMate(null)
      }
      setStats({ depth: currentDepth, nodes: r.nodes, nps: r.nps, elapsed_ms: r.elapsed_ms, pv: r.pv, startFen: searchFen })
      setLastMove({ from: r.best_move.slice(0, 2), to: r.best_move.slice(2, 4) })
      chess.current.move(r.best_move)
      syncState()
      if (chess.current.isGameOver()) setGameStatus(gameOverMsg())
    } catch (err) {
      console.error('Engine error:', err)
    } finally {
      setIsThinking(false)
    }
  }, [syncState, gameOverMsg])

  function onDrop(source: Square, target: Square): boolean {
    if (isThinking || chess.current.isGameOver() || isEngineTurn()) return false
    try { chess.current.move({ from: source, to: target, promotion: 'q' }) } catch { return false }
    setLastMove({ from: source, to: target })
    syncState()
    if (chess.current.isGameOver()) { setGameStatus(gameOverMsg()); return true }
    runEngineMove(depth)
    return true
  }

  function newGame() {
    chess.current.reset()
    syncState()
    setGameStatus(null); setEvalCp(null); setEvalMate(null)
    setStats(null); setLastMove(null)
    setBoardOrientation(playerColor)
    if (playerColor === 'black') setTimeout(() => runEngineMove(depth), 50)
  }

  const inCheck     = chess.current.inCheck()
  const squareStyles = buildSquareStyles(lastMove, inCheck, inCheck ? getKingSquare(chess.current) : null)

  return (
    <main className="flex gap-6 p-6 justify-center items-start flex-1 flex-wrap">

      {/* ── Board column ─────────────────────────────────── */}
      <div className="flex flex-col items-center gap-3">

        {/* Thinking badge - always rendered so it never shifts the board */}
        <div className={[
          'flex items-center gap-2 px-4 py-1.5 rounded-full border border-accent',
          'bg-surface text-accent text-sm font-semibold transition-opacity duration-200',
          isThinking ? 'opacity-100' : 'opacity-0 pointer-events-none',
        ].join(' ')}>
          <span className="w-2 h-2 rounded-full bg-accent animate-pulse" />
          Engine is thinking…
        </div>

        {/* Eval bar + board */}
        <div className="flex gap-3 items-start">
          <EvalBar scoreCp={evalCp} scoreMate={evalMate} isWhitePerspective size={boardSize} />
          <div className="select-none relative">
            {gameStatus && (
              <div className="absolute inset-0 z-10 flex items-center justify-center rounded bg-bg/80 backdrop-blur-sm">
                <div className="bg-surface border border-accent rounded-2xl px-8 py-5 text-base font-bold text-copy text-center shadow-2xl">
                  {gameStatus}
                </div>
              </div>
            )}
            <Chessboard
              id="play-board"
              position={fen}
              onPieceDrop={onDrop}
              boardOrientation={boardOrientation}
              boardWidth={boardSize}
              customDarkSquareStyle={{ backgroundColor: '#b58863' }}
              customLightSquareStyle={{ backgroundColor: '#f0d9b5' }}
              customSquareStyles={squareStyles}
              arePiecesDraggable={!isThinking && !chess.current.isGameOver()}
            />
          </div>
        </div>
      </div>

      {/* ── Sidebar ──────────────────────────────────────── */}
      <aside className="flex flex-col gap-3 w-56">
        <MoveHistory moves={moves} />
        <GameControls
          depth={depth}
          onDepthChange={setDepth}
          playerColor={playerColor}
          onPlayerColorChange={setPlayerColor}
          onNewGame={newGame}
          onFlipBoard={() => setBoardOrientation(o => o === 'white' ? 'black' : 'white')}
          onGetFen={() => chess.current.fen()}
          onGetPgn={() => chess.current.pgn()}
          disabled={moves.length > 0}
        />
        <EngineStats
          depth={stats?.depth ?? null}
          nodes={stats?.nodes ?? null}
          nps={stats?.nps ?? null}
          elapsed_ms={stats?.elapsed_ms ?? null}
          pv={stats?.pv ?? []}
          startFen={stats?.startFen}
          showPv={false}
        />
      </aside>
    </main>
  )
}
