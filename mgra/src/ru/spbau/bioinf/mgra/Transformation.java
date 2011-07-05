package ru.spbau.bioinf.mgra;

import org.jdom.Element;
import java.util.ArrayList;
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
        List<Integer> ids = new ArrayList<Integer>();
        for (int i = 0; i < all.size(); i++) {
            Chromosome chromosome =  all.get(i);
            for (End end : ends) {
                if (chromosome.contains(end)) {
                    this.chromosomes.add(chromosome);
                    ids.add(i);
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

        List<Chromosome> parts = new ArrayList<Chromosome>();
        for (Chromosome chromosome : chromosomes) {
            chromosome.split(parts);
        }

        for (int i = 0; i < parts.size(); i++) {
            Chromosome first = parts.get(i);
            for (int j = i+1; j < parts.size(); j++) {
                Chromosome second = parts.get(j);
                if (first.join(second)) {
                    parts.remove(j);
                    j = i;
                }
            }
        }

        int order = 0;
        while (parts.size() > order && ids.size() > order) {
            Chromosome chr = parts.get(order);
            int id = ids.get(order);
            chr.setId(id + 1);
            all.remove(id);
            all.add(id, chr);
            order++;
        }
        if (parts.size() > order) {
            Chromosome chr = parts.get(order);
            chr.setId(all.size() +1);
            all.add(chr);
        }
        if (ids.size() > order) {
            int id = ids.get(order);
            all.remove(id);
        }

        for (Chromosome chromosome : parts) {
             chromosome.clearEnds();
             for (End end : ends) {
                 chromosome.mark(end);
             }
             after.addContent(chromosome.toXml());
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
