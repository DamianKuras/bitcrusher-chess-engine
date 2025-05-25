import chess
import subprocess
import random


def get_engine_moves(fen, engine_path):
    command = f"position fen {fen}\n" \
              f"go perft \n"
    result = subprocess.run([engine_path],input=command, capture_output=True, text=True)
    # Engine should output moves as UCI strings separated by spaces
    return set(result.stdout.strip().split())


def get_pythonchess_moves(fen):
    board = chess.Board(fen)
    return {move.uci() for move in board.legal_moves}


def find_move_discrepancy(engine_path):
    count =0
    while True:
        if count % 100==0:
            print(count)
        count+=1
        # Generate random position
        board = chess.Board()
        for _ in range(random.randint(0, 40)):
            if not board.legal_moves:
                break
            board.push(random.choice(list(board.legal_moves)))

        fen = board.fen()

        # Get moves from both bitcrusher engine and python chess
        engine_moves = get_engine_moves(fen, engine_path)
        py_moves = get_pythonchess_moves(fen)

        # Find discrepancies
        extra_moves = engine_moves - py_moves
        missing_moves = py_moves - engine_moves
        required_moves_str = "{" + ", ".join(f'"{move}"' for move in sorted(py_moves)) + "}"
        print("Required moves:", required_moves_str)
        if extra_moves or missing_moves:
            print(f"Mismatch found in FEN: {fen}")
            if extra_moves:
                print(f"Engine has extra moves: {', '.join(sorted(extra_moves))}")
            if missing_moves:
                print(f"Engine missing moves: {', '.join(sorted(missing_moves))}")
            return

      
if __name__ == "__main__":
    find_move_discrepancy("./bin/Release/Uci")
