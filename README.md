# 4D-Chess
Examples of 4D chess and other variants I've invented and implemented. The code is private for now.

## List of features - variants are defined using any combination of these features
- Any number of dimensions - 2,3,4,14,anything (actually 14 is the limit)
- Any side lengths - Chess on a 6x3x4x2x5 hyperrectangle, why not? Also: why?
- Any dimensions wrapping around the edge - move one step forward from D8 and arrive on D1
- "Forward" and "non-forward" dimensions. Pawns normally move forward in forward dimensions, and capture diagonally on step forward in forward dimensions and one step in non-forward dimension.
- Pawns moving in specified direction. Forward/backwards in forward dimensions, or forward/backwards in non-forward dimensions (ie right/left).
- Any number of moves per turn for each player. Allow or don't allow moving the same piece twice in one turn.
- Multiple different win conditions - Checkmate, king captured, king arrived to target square
- Gravity - pieces fall down in the direction of gravity
- Invincible king - king can't capture or be captured. Win by reaching target square (or any other win condition, if you so choose)
- Define where pawns promote. Define which pieces other than pawns that can promote

## List of upcoming features
- Non-(hyper)rectangle board shapes - circle, tetrahedon, whatever
- Parts of the board where only one player is allowed to be
- Parts of the board where only one player is allowed to capture pieces
- More than two players
- Simultaneous moves
- Capture the flag feature
- Tower defence feature - don't let the opponents pieces get through
- Other win conditions
- Variant maker - define the board shape and rules yourself
- Engine that can play any variant
- Proper UI and game selection

## Examples of variants
![alt text](github_images/10x10_chess.png)
