package ru.spbau.bioinf.mgra;

import org.jdom.Element;

import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

public class Transformation {
    private List<Chromosome> chromosomes = new ArrayList<Chromosome>();
    private Element before = new Element("before");
    private Element after = new Element("after");

    End[] ends = new End[4];

    public Transformation(String text) {
        String[] data = text.split("[ \t]");
        for (int i = 0; i < ends.length; i++) {
            ends[i] = new End(i, data[i]);
        }
    }

    public void update(Genome genome) {
        List<Chromosome> all = genome.getChromosomes();
        for (Chromosome chromosome : all) {
            for (End end : ends) {
                if (chromosome.contains(end)) {
                    this.chromosomes.add(chromosome);
                    break;
                }
            }
        }
        for (Chromosome chromosome : chromosomes) {
            chromosome.clearEnds();
            for (End end : ends) {
                chromosome.mark(end);
            }
            before.addContent(chromosome.toXml());
        }
        if (chromosomes.size() ==1) {
            Chromosome chr = chromosomes.get(0);
            List<Gene> genes = chr.getGenes();
            LinkedList<Position> positions = new LinkedList<Position>();
            for (int i = 0; i < genes.size(); i++) {
                Gene gene = genes.get(i);
                gene.addPositions(positions, i);
            }
            Collections.sort(positions);
            Position first = positions.getFirst();
            if (first.getPos() == 0 && first.getSide() == -1) {
                positions.addFirst(new Position(-1,1, 0));
            }
            if (positions.size() > 2) {
                reverseOrFision(all, genes, positions);
                after.addContent(chr.toXml());
            }
            chr.clearEnds();
        }
    }

    private void reverseOrFision(List<Chromosome> all, List<Gene> genes, LinkedList<Position> positions) {
        Position left = positions.get(1);
        Position right = positions.get(2);

        int start = left.getPos();
        int end = right.getPos();
        if (left.getColor() % 2 != right.getColor() %2) {
            List<Gene> reverse = new ArrayList<Gene>();
            for (int pos = end; pos >=start; pos--) {
                Gene gene = genes.get(pos);
                gene.reverse();
                reverse.add(gene);
                genes.remove(pos);
            }
            for (int i = 0; i < reverse.size(); i++) {
                genes.add(start + i, reverse.get(i));
            }
        } else {
            List<Gene> cut = new ArrayList<Gene>();
            for (int pos = start; pos <=end; pos++) {
                cut.add(genes.get(start));
                genes.remove(start);
            }
            Chromosome add = new Chromosome(all.size() + 1, cut);
            all.add(add);
            after.addContent(add.toXml());
            add.clearEnds();
        }
    }

    public Element toXml() {
        Element tr = new Element("transformation");
        tr.addContent(before);
        for (End end : ends) {
            tr.addContent(end.toXml());
        }
        tr.addContent(after);
        return tr;
    }

}
