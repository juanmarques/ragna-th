# Auditoria de Gaps do Jogo (Ragna-TH)

## Escopo da auditoria
Esta auditoria compara o plano oficial do projeto com os itens ainda abertos no roadmap e os TODOs de código que impactam experiência de jogo, fidelidade ao Ragnarok clássico e prontidão para produção.

Referências usadas:
- `ROADMAP.md` (itens não concluídos)
- `CHANGELOG.md` (limitações explicitamente adiadas)
- varredura de TODOs no código-fonte (`RagnarokUE/Source`)

---

## Resumo executivo

- **14 gaps funcionais abertos** no roadmap atual.
- **5 gaps de produção/infra** ainda marcados como TODO no código (persistência, anti-cheat DB, multi-server, VFX, wiring final de classes/UI).
- **Maior risco de curto prazo**: divergência de fórmulas de combate (ATK min/max; Hard/Soft DEF; HP/SP por nível), pois afeta diretamente balanceamento e “feeling” de combate.
- **Maior risco sistêmico**: ausência de persistência robusta para guild e sistemas de storage (Kafra/Guild), bloqueando progressão social de longo prazo.

---

## Gaps priorizados

## P0 — Críticos para gameplay/base RO

1. **Weapon ATK min/max rolling** (incluindo crítica usando valor máximo)
   - Impacto: dano inconsistente com RO clássico e percepção de combate “errada”.
   - Fonte: roadmap aberto.

2. **Separação Hard DEF vs Soft DEF na execução GAS**
   - Impacto: redução de dano incorreta, afetando PvE/PvP e tuning global.
   - Fonte: roadmap aberto.

3. **Tabela HP/SP por nível (não apenas modificador simples por job)**
   - Impacto: progressão de sobrevivência/mana fora da curva esperada.
   - Fonte: roadmap aberto.

4. **War of Emperium completo** (captura/defesa/regras essenciais)
   - Impacto: ausência de um dos loops endgame centrais do jogo.
   - Fonte: roadmap aberto.

## P1 — Alto impacto social/economia/retenção

5. **Persistência de dados de guild**
   - Impacto: risco de perda de progresso social e inconsistência entre sessões.
   - Fonte: roadmap aberto.

6. **Guild emblem system**
   - Impacto: identidade social incompleta para guildas.
   - Fonte: roadmap aberto + changelog adiado por dependência de pipeline gráfico.

7. **Kafra storage (conta)**
   - Impacto: limita economia e qualidade de vida dos jogadores.
   - Fonte: roadmap aberto.

8. **Guild storage compartilhado**
   - Impacto: bloqueia organização de recursos por guilda.
   - Fonte: roadmap aberto.

9. **Cart inventory (classe Merchant)**
   - Impacto: reduz profundidade de gameplay de Merchant/Blacksmith.
   - Fonte: roadmap aberto.

## P2 — Consistência de mundo e conteúdo

10. **Day/Night spawn support**
    - Impacto: ecossistema de monstros menos dinâmico.
    - Fonte: roadmap aberto.

11. **Per-spawn cell randomization**
    - Impacto: padrões de spawn previsíveis; menor variação de farming.
    - Fonte: roadmap aberto.

12. **Multi-hit attack support**
    - Impacto: skills/passivas icônicas incompletas (Double/Triple Attack etc.).
    - Fonte: roadmap aberto.

13. **Refine bonus por nível de arma (item pendente/duplicado no roadmap)**
    - Impacto: há ambiguidade de tracking (aparece em mais de uma fase).
    - Fonte: roadmap aberto (duplicidade indica gap de governança do backlog).

## P3 — Prontidão técnica / produção

14. **TODOs de infraestrutura ainda em aberto**
    - Anti-cheat com logging em banco (produção).
    - Suporte multi-server no subsystem de rede.
    - VFX de clima (weather) dependente de assets.
    - Binding final de `DefaultPawnClass` e `HUDClass` no `GameMode`.
    - Upgrade de hash de senha para algoritmo forte (bcrypt/argon2) para produção.

---

## Gap de processo (backlog hygiene)

- Há **item potencialmente duplicado de refine bonus** em fases diferentes do roadmap.
- Recomendação: consolidar em um único item com critério de aceite explícito (fórmula, testes unitários e validação in-game).

---

## Plano recomendado (2 sprints)

### Sprint 1 (foco em fidelidade de combate)
- Fechar P0.1, P0.2 e P0.3.
- Adicionar testes automatizados de fórmula (golden tests com vetores conhecidos rAthena).
- Revisar item de refine e remover duplicidade do roadmap.

### Sprint 2 (foco em retenção social)
- Fechar P1.5, P1.7, P1.8 e P1.9 (persistência guild + storages + cart).
- Entregar MVP de WoE operacional (regras mínimas + persistência de ownership).

---

## Critérios de aceite sugeridos

- **Combate**: dano médio e distribuição por classe/arma dentro de tolerância definida contra baseline rAthena.
- **Persistência**: dados de guild/storage íntegros após reinício do servidor.
- **Social**: criação, gerenciamento e identificação visual de guild funcional (incl. emblema).
- **Mundo**: spawns com variação temporal/espacial verificável por telemetria.

