package ru.spbau.bioinf.mgra;

public class Position implements Comparable<Position> {
    private int pos;
    private int side;
    private int color;


    public Position(int pos, int side, int color) {
        this.pos = pos;
        this.side = side;
        this.color = color;
    }

    public int compareTo(Position p) {
        if (p.pos != pos) {
            return pos - p.pos;
        }
        return side - p.side;
    }

    public int getPos() {
        return pos;
    }

    public int getSide() {
        return side;
    }

    public int getColor() {
        return color;
    }
}
