package main;

import builders.BlockBuilder;
import builders.ForBuilder;
import builders.IfElseBuilder;
import expressions.*;
import instructions.Print;

import java.util.*;

// Press Shift twice to open the Search Everywhere dialog and type `show whitespaces`,
// then press Enter. You can now see whitespace characters in your code.
public class Main {
	public static void main(String[] args) {
		boolean d1 = false;
		boolean d2 = true;

		try {
			System.out.println("The program from Moodle:");

			var program = new BlockBuilder()
				.declareVariable('x', Literal.of(101))
				.declareVariable('y', Literal.of(1))
				.declareProcedure("out", List.of('a'), new BlockBuilder()
					.print(Sum.of(Variable.named('a'), Variable.named('x')))
					.build()
				)
				.assign('x', Diff.of(Variable.named('x'), Variable.named('y')))
				.invoke("out", List.of(Variable.named('x')))
				.invoke("out", List.of(Literal.of(100)))
				.block(new BlockBuilder()
					.declareVariable('x', Literal.of(10))
					.invoke("out", List.of(Literal.of(100)))
					.build()
				)
				.build();

			program.startProgram(d1);
		}
		catch (Exception e) {
			System.out.println(e.getMessage());
		}

		try {
			System.out.println("\nDifferent program:");

			var program = new BlockBuilder()
				.declareVariable('x', Literal.of(101))
				.declareVariable('y', Literal.of(1))
				.declareProcedure("out", List.of('a'), new BlockBuilder()
					.print(Sum.of(Variable.named('a'), Variable.named('x')))
					.build()
				)
				.assign('x', Diff.of(Variable.named('x'), Variable.named('y')))
				.invoke("out", List.of(Variable.named('x')))
				.invoke("out", List.of(Literal.of(100)))
				.block(new BlockBuilder()
					.declareVariable('x', Literal.of(10))
					.invoke("out", List.of(Literal.of(100)))
					.build()
				)
				.ifElse(new IfElseBuilder(Variable.named('x'), ">=", Literal.of(100))
					.thenInstruction(new Print(Variable.named('x')))
					.otherwiseInstruction(new Print(Literal.of(-2)))
					.thenInstruction(new Print(Literal.of(2)))
					.otherwiseInstruction(new Print(Literal.of(-3)))
					.build()
				)
				.forLoop(new ForBuilder('i', Literal.of(4))
					.invoke("out", List.of(Variable.named('i')))
					.print(Quotient.of(Variable.named('i'), Literal.of(2)))
					.ifElse(new IfElseBuilder(Variable.named('i'), ">", Literal.of(2))
						.otherwiseInstruction(new BlockBuilder()
							.declareProcedure("g", Arrays.asList('i', 'j'), new BlockBuilder()
								.print(Diff.of(Variable.named('i'), Variable.named('j')))
								.build()
							)
							.invoke("g", Arrays.asList(Variable.named('y'), Variable.named('i')))
							.build()
						)
						.build()
					)
					.build()
				)
				.build();

			program.startProgram(d2);
		}
		catch (Exception e) {
			System.out.println(e.getMessage());
		}
	}
}
